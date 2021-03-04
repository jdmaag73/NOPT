//
//                                   
// NOPTb-fair replacement policy 
// Javier Diaz, jdmaag@gmail.com 
// Pablo Ibañez, imarin@unizar.es
//                                   
// This software and the methods and concepts it implements are available without charge to anyone for academic, research, experimental or personal use. 
// If you wish to distribute or make other use of the software and/or the methods, you may purchase a license to do so from the authors.
//

#ifdef CRC2N
#include "../inc/champsim_crc2n.h"
#else
#include "../inc/champsim_crc2.h"
#endif

#include <bitset>
#include <iostream>
#include <fstream>
#include <list>
#include <chrono>
#include <ctime>
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/vector.hpp"

#ifndef NUM_CORE
#define NUM_CORE 1
#endif

#ifndef SETS_PER_CORE
#define SETS_PER_CORE 2048
#endif

#define LLC_SETS NUM_CORE*SETS_PER_CORE
#define LLC_WAYS 16

#define INI_ACCESOS 1000
#define MAX_BUSQUEDA 1000

#ifndef MAX_ACCESOS_ESCALADO
#define MAX_ACCESOS_ESCALADO 1000
#endif

#ifndef BYPASS
#define BYPASS 0
#endif

#define TRAZA(set) (set==1010)
//#define TRAZA(set) (1)

class set_accesses {
public:
	uint32_t idx;
	uint32_t max_idx;
	vector<uint64_t> bloque;
	vector<uint32_t> instr_count;
	
	set_accesses() {
		bloque.reserve(INI_ACCESOS);
		instr_count.reserve(INI_ACCESOS);
	};

	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		ar & max_idx;
		ar & bloque;
		ar & instr_count;
    }

};

class cache_accesses {
public:
	set_accesses sets[LLC_SETS];
	
	cache_accesses() {};
	
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & sets;
    }
};

cache_accesses OPT_acceses[NUM_CORE];

uint8_t 	simulando=0;
uint64_t	warmup_inst=0, simulation_inst=0, total_inst=0, low_thrshld=0, high_thrshld=0; 

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize OPT_accesses, BYPASS is " << BYPASS << endl;

	for (uint32_t cpu=0; cpu<NUM_CORE; cpu++)
	{
		for (uint32_t i=0; i<LLC_SETS; i++) {
			OPT_acceses[cpu].sets[i].idx=0;
			OPT_acceses[cpu].sets[i].max_idx=0;
		}
	}
	
	simulando = std::stoi(getenv("SIMULANDO"));
	
	if (simulando == 0)
	{
		cout << "Primera vuelta, escribire a fichero " << getenv("FICH_OPT_ACCESSES") << endl;
	}
	else if (simulando == 1)
	{
		cout << "Segunda vuelta, leo fichero " << getenv("FICH_OPT_ACCESSES") << endl;
		
		// create and open a binary archive for input
		std::ifstream ifs(getenv("FICH_OPT_ACCESSES"), std::ios::binary);
		boost::archive::binary_iarchive ia(ifs);

		cout << "Comienzo lectura de OPT_acceses[] a " 
			<< std::ctime(new time_t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))) << endl;
		
        // read class instance from archive
		for (uint32_t cpu=0; cpu<NUM_CORE; cpu++)
		{
			ia >> OPT_acceses[cpu];
		}

		cout << "Acabo lectura de OPT_acceses[] a " 
			<< std::ctime(new time_t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))) << endl;	

		warmup_inst = std::stoi(getenv("WARMUP_INSTRUCTIONS"));
		simulation_inst = std::stoi(getenv("SIMULATION_INSTRUCTIONS"));
		total_inst = warmup_inst + simulation_inst;
		low_thrshld = total_inst / 3;
		high_thrshld = low_thrshld * 2;
		cout << "warmup_inst " << warmup_inst << " simulation_inst " << simulation_inst 
			 << " total_inst " << total_inst << " low_thrshld " << low_thrshld << " high_thrshld " << high_thrshld << endl;	
		
		// El contador de instrucciones es de 32 bits...
		assert(total_inst < std::numeric_limits<uint32_t>::max());
	}	
	else
	{
		assert(0);
	}
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
	struct tmpOPT {
		uint64_t bloque;
		uint32_t cpu;
		bool acceso_encontrado;
	};

	struct tmpOPT tmpOPTset[LLC_WAYS+BYPASS];

	uint32_t bypass_posible = 0;
    uint32_t via = 666;
	uint64_t block = paddr >> 6; // assuming 64B line size, get rid of lower bits

	if (TRAZA(set)) 
		cout << "Set " << set << " GetVictimInSet para addr " << std::hex << paddr << std::dec 
			 << " bloque " << std::hex << block << std::dec << " en cpu " << cpu << " instr " << get_instr_count(cpu) 
			 << " ciclo " << get_cycle_count() << " simulando " << (int)simulando << endl;
    
	if (BYPASS && (type != WRITEBACK))
		bypass_posible = 1;

	// En la primera vuelta hacemos bypass de todo lo que podemos, y si no a la via 0
	if (simulando == 0)
	{
		if (bypass_posible)
			via = 16;
		else
			via = 0;

		return via;
	}
	
	// Para cada bloque valido que ahora mismo tenemos en el set, buscamos cuando se accedera por primera vez
	// En caso de que sea posible el Bypass, añado tambien a la lista de busqueda el bloque que estoy accediendo
	uint8_t num_por_encontrar[NUM_CORE];
	memset(num_por_encontrar, '\0', sizeof(num_por_encontrar));

	if (TRAZA(set)) 
		cout << "Set " << set << ": elementos ";
		
	for (uint32_t i=0; i<LLC_WAYS; i++)
	{
		if (current_set[i].valid)
		{
			tmpOPTset[i].bloque = current_set[i].address;
			tmpOPTset[i].cpu = current_set[i].cpu;
			num_por_encontrar[tmpOPTset[i].cpu]++;
			tmpOPTset[i].acceso_encontrado = false;

			if (TRAZA(set)) 
				cout << i << "=" << std::hex << tmpOPTset[i].bloque << std::dec << " (" << tmpOPTset[i].cpu << ") ";
		}
		else
		{
			// Echamos un bloque no valido
			if (TRAZA(set)) 
				cout << endl << "Set " << set << ": El elemento " << i << " es no valido, coloco ahi el bloque " << std::hex << block << std::dec << endl;
			return i;
		}
	}

	if (TRAZA(set)) 
		cout << endl;

	if (bypass_posible)
	{
		tmpOPTset[LLC_WAYS].bloque = block;
		tmpOPTset[LLC_WAYS].cpu = cpu;
		num_por_encontrar[cpu]++;
		tmpOPTset[LLC_WAYS].acceso_encontrado = false;
	}

	int pos_max=-1;
	double offset_escalado_max=-1;
	
	for (uint32_t core=0; core<NUM_CORE; core++)
	{
		int pos_max_core=-1;
		long long int offset=-1;
		
		for (uint32_t i=OPT_acceses[core].sets[set].idx, l=0; i<OPT_acceses[core].sets[set].max_idx && num_por_encontrar[core]>0 && l < MAX_BUSQUEDA; i++, l++)
		{
			for (uint32_t j=0; j<LLC_WAYS+bypass_posible; j++)
			{
				if (   ((tmpOPTset[j].acceso_encontrado == false) && (tmpOPTset[j].cpu == core) && (tmpOPTset[j].bloque == OPT_acceses[core].sets[set].bloque[i]))
					&& !((j==LLC_WAYS) && (core==cpu) && (i==OPT_acceses[core].sets[set].idx)))
				{
					tmpOPTset[j].acceso_encontrado = true;
					num_por_encontrar[core]--;
					pos_max_core = j;
					
					offset = i - OPT_acceses[core].sets[set].idx +1;

					if (TRAZA(set)) 
						cout << "Set " << set << ": Bloque " << std::hex << tmpOPTset[j].bloque << std::dec << " en pos " << j << " accedido en idx " << i 
							 << " de core " << core << ", offset " << offset << endl;
				}
			}
		}
		
		// Busco si hay alguno sin acceso futuro
		if (num_por_encontrar[core]>0)
		{
			for (uint32_t i=0; i<LLC_WAYS+bypass_posible; i++)
			{
				if ((tmpOPTset[i].acceso_encontrado == false) && (tmpOPTset[i].cpu == core))
				{
					if (TRAZA(set)) 
						cout << "Set " << set << ": Echamos bloque sin acceso de core " << core << " en pos " << i << endl;

					return i;
				}
			}
			assert(0);
		}

		// Miro si el acceso mas tardío de este core es mas tarde que el que ya tenia de otros cores
		if ((offset > -1) && (offset>offset_escalado_max))
		{
			offset_escalado_max = offset;
			pos_max=pos_max_core;
		}
	}
	
	if (TRAZA(set)) 
		cout << "Set " << set << ": Echamos bloque mas tarde a acceder en pos " << pos_max << endl;
	
    return pos_max;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
	uint64_t ic = get_instr_count(cpu);

	int iHit = 0;
	if (hit)
		iHit = 1;

	uint64_t block = paddr >> 6; // assuming 64B line size, get rid of lower bits
	
	if (TRAZA(set)) 
		cout << "Set " << set << " UpdateReplacementState way " << way << " addr " << std::hex << paddr << " block " << block  << std::dec
			 << " type " << type << " hit " << iHit << " en core " << cpu << " instr " << ic << endl;
	
	if (type == WRITEBACK)
		return;

	if (simulando == 0)
	{
		// En la primera "vuelta", recordamos los bloques 

		if (TRAZA(set)) 
			cout << "Set " << set << ": Meto acceso bloque " << std::hex << block << std::dec << " a cpu " << cpu << " idx " << OPT_acceses[cpu].sets[set].idx << endl;
				
		OPT_acceses[cpu].sets[set].bloque.push_back(block);
		OPT_acceses[cpu].sets[set].instr_count.push_back(ic);
		OPT_acceses[cpu].sets[set].idx++;
	}
	else
	{
		// En la segunda "vuelta", vamos avanzando los indices de cada set con cada acceso
		
		// Solo hemos guardado unos <total_inst> instrucciones, así que volvemos al inicio si hemos pasado ese valor
		if (ic >= total_inst)
		{
			ic = ic % total_inst;
			if ((ic < low_thrshld) && (OPT_acceses[cpu].sets[set].idx > 1) && (OPT_acceses[cpu].sets[set].instr_count[OPT_acceses[cpu].sets[set].idx-1] > high_thrshld))
			{
				if (TRAZA(set)) 
					cout << "Set " << set << " cpu " << cpu << ": Reinicio lista de bloques"  << endl;
				OPT_acceses[cpu].sets[set].idx = 0;
			}
		}
		
		while (OPT_acceses[cpu].sets[set].idx < OPT_acceses[cpu].sets[set].max_idx)
		{
			if (OPT_acceses[cpu].sets[set].bloque[OPT_acceses[cpu].sets[set].idx] == block)
			{
				if (TRAZA(set)) 
					cout << "Set " << set << ": Acceso esta en cpu " << cpu << " idx " << OPT_acceses[cpu].sets[set].idx  << endl;
				OPT_acceses[cpu].sets[set].idx++;
				break;
			}
			else if (OPT_acceses[cpu].sets[set].instr_count[OPT_acceses[cpu].sets[set].idx] > ic)
			{
				if (TRAZA(set)) 
					cout << "Set " << set << ": Llego en cpu " << cpu << " a instr " << OPT_acceses[cpu].sets[set].instr_count[OPT_acceses[cpu].sets[set].idx] << " y no lo he encontrado "  << endl;
				break;
			}
			else
			{
				if (TRAZA(set)) 
					cout << "Set " << set << ": Me salto acceso en cpu " << cpu << " idx " << OPT_acceses[cpu].sets[set].idx << endl;
				OPT_acceses[cpu].sets[set].idx++;
			}
		}
	}
	
	return;
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{
	if (simulando == 0)
	{
		// Fin de la primera vuelta, volcamos a fichero OPT_acceses
		for (uint32_t cpu=0; cpu < NUM_CORE; cpu++)
		{
			for (uint32_t i=0; i<LLC_SETS; i++) {
				OPT_acceses[cpu].sets[i].max_idx = OPT_acceses[cpu].sets[i].idx;
				OPT_acceses[cpu].sets[i].idx = 0;
			}
		}
		
		// create/overwrite and open a binary archive for output
		std::ofstream ofs(getenv("FICH_OPT_ACCESSES"), std::ios::binary | std::ios::trunc);
		boost::archive::binary_oarchive oa(ofs);
		
        // write class instance to archive
		for (uint32_t cpu=0; cpu<NUM_CORE; cpu++)
		{
			oa << OPT_acceses[cpu];		
		}

		cout << "Fichero de volcado de OPT_acceses[] creado" << endl;
	}
}

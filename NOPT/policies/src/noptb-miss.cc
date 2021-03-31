//
//                                   
// NOPTb-miss replacement policy 
// Javier Diaz, jdmaag@gmail.com 
// Pablo Ibañez, imarin@unizar.es
//                                   
// This software and the methods and concepts it implements are available without charge to anyone for academic education, academic research, experimental or personal use. 
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
#include <sstream>
#include <list>
#include <chrono>
#include <ctime>
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/vector.hpp"

#ifndef NUM_CORE
#define NUM_CORE 1
#endif

#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

#define INI_ACCESOS 1000
#define MAX_BUSQUEDA 1000

#ifndef MAX_ACCESOS_ESCALADO
#define MAX_ACCESOS_ESCALADO 1000
#endif

#ifndef BYPASS
#define BYPASS 0
#endif

#ifndef maxRRPV
#define maxRRPV 3
#endif

#ifndef SIM0_RANDOM
#ifndef SIM0_TODO_FALLOS
#define SIM0_SRRIP
#endif
#endif

uint32_t rrpv[LLC_SETS][LLC_WAYS];

//#define TRAZA(set) (set==7046)
#define TRAZA(set) (0)

#ifdef DEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x) if (0) x
#endif

vector<uint32_t> instr_count32;

class set_accesses {
public:
	uint32_t idx;
	uint32_t max_idx;
	uint64_t previdx_access_cycle;
	vector<uint64_t> bloque;
	vector<uint64_t> instr_count;
	vector<uint32_t> cycle_distance_previdx;

	set_accesses() {
		bloque.reserve(INI_ACCESOS);
		instr_count.reserve(INI_ACCESOS);
		cycle_distance_previdx.reserve(INI_ACCESOS);
	};

	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		// note, version is always the latest when saving
		
		ar & max_idx;
		ar & bloque;
		if (version == 0) {
			ar & instr_count32;
			instr_count.reserve(instr_count32.size());
			instr_count.assign(instr_count32.begin(), instr_count32.end());
			instr_count32.clear();
		} else {
			ar & instr_count;
		}
		ar & cycle_distance_previdx;
    }
};

// En la versión 0, instr_count es de 32 bits
BOOST_CLASS_VERSION(set_accesses, 1)

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

cache_accesses OPT_accesses[NUM_CORE], OPT_accesses_sig[NUM_CORE];

int    		simulando=0;
uint64_t	warmup_inst=0, simulation_inst=0, total_inst=0, trace_inst=0, extra_inst=0; 
uint64_t    last_warmup_inst[NUM_CORE];
bool        warmup_finished=false, measure_finished=false;

int 		factor_core[NUM_CORE];	

uint64_t    found=0, notfound=0, skip=0, missing_measure=0, missing_extra=0;

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize OPT_accesses, BYPASS is " << BYPASS << endl;

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            rrpv[i][j] = maxRRPV;
        }
    }

	for (uint32_t cpu=0; cpu<NUM_CORE; cpu++)
	{
		for (uint32_t i=0; i<LLC_SETS; i++) {
			OPT_accesses[cpu].sets[i].idx=0;
			OPT_accesses[cpu].sets[i].max_idx=0;
			OPT_accesses[cpu].sets[i].previdx_access_cycle=0;
			OPT_accesses_sig[cpu].sets[i].idx=0;
			OPT_accesses_sig[cpu].sets[i].max_idx=0;
			OPT_accesses_sig[cpu].sets[i].previdx_access_cycle=0;
		}		
		last_warmup_inst[cpu]=0;
	}

	for (uint32_t cpu=0; cpu<NUM_CORE; cpu++) 
	{
		factor_core[cpu] = 0;
	}
	
	simulando = std::stoi(getenv("SIMULANDO"));
	
	if (simulando == 0)
	{
		cout << "Primera iteracion, escribire a fichero " << getenv("FICH_OPT_ACCESSES_SIG") << endl;
	}
	else if (simulando > 0)
	{
		cout << "Siguientes iteraciones, leo fichero " << getenv("FICH_OPT_ACCESSES") << " y escribo en " << getenv("FICH_OPT_ACCESSES_SIG") << endl;

		// create and open a binary archive for input
		std::ifstream ifs(getenv("FICH_OPT_ACCESSES"), std::ios::binary);
		boost::archive::binary_iarchive ia(ifs);

		cout << "Comienzo lectura de OPT_accesses[] a " 
			<< std::ctime(new time_t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))) << endl;
		
        // read class instance from archive
		for (uint32_t cpu=0; cpu<NUM_CORE; cpu++)
		{
			ia >> OPT_accesses[cpu];
		}

		cout << "Acabo lectura de OPT_accesses[] a " 
			<< std::ctime(new time_t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))) << endl;	

	}
	
#ifdef SIM0_RANDOM
	cout << "Base es RANDOM" << endl;
#elif defined SIM0_TODO_FALLOS
	cout << "Base es TODO_FALLOS" << endl;
#else
	cout << "Base es SRRIP con maxRRPV " << maxRRPV << endl;
#endif

	warmup_inst = std::stoi(getenv("WARMUP_INSTRUCTIONS"));
	simulation_inst = std::stoi(getenv("SIMULATION_INSTRUCTIONS"));
	total_inst = warmup_inst + simulation_inst;
	trace_inst = std::stoi(getenv("TRACE_INSTRUCTIONS"));
	if (getenv("EXTRA_INSTRUCTIONS") != NULL) {
	  extra_inst = std::stoi(getenv("EXTRA_INSTRUCTIONS"));
	} else {
		extra_inst = 0;
	}
	cout << "warmup_inst " << warmup_inst << " simulation_inst " << simulation_inst << " total_inst " << total_inst
		  << " trace_inst " << trace_inst << " extra_inst " << extra_inst << endl;	
		
	// El contador de instrucciones es de 32 bits...
	assert(total_inst < std::numeric_limits<uint32_t>::max());	
	
#ifdef EXPLORAR
	// Los mixes son en realidad de las mismas trazas pero vamos a variar la "distancia"
	// en un 50%, 300% y 900% (valores fijos) para hasta 3 de las trazas. El objetivo
	// es explorar el espacio cerca del mínimo local en que se pudiera estar
	assert(getenv("MIX")!=NULL && ((string)getenv("MIX")).length() > 0);
	assert(NUM_CORE==4);
	int mix = std::stoi(getenv("MIX"));
	int i=0;
	short int opcs[] = {0, 50, 300, 900};
	
	for (int c0=0; c0<NUM_CORE; c0++) {
		for (int c1=0; c1<NUM_CORE; c1++) {
			for (int c2=0; c2<NUM_CORE; c2++) {
				for (int c3=0; c3<NUM_CORE; c3++) {
					if (c0==0 || c1==0 || c2==0 || c3==0) {
						if (i == mix) {
							factor_core[0] = opcs[c0];
							factor_core[1] = opcs[c1];
							factor_core[2] = opcs[c2];
							factor_core[3] = opcs[c3];
							
							goto fin;
						}
						i++;
					}  
				}
			}
		}
	}

	fin:
	cout << "Factores core: 0=" << factor_core[0]
		<< " c1=" << factor_core[1]
		<< " c2=" << factor_core[2]
		<< " c3=" << factor_core[3] << endl;
	
#endif
	
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
	uint64_t block = paddr >> 6; // assuming 64B line size, get rid of lower bits

	if (TRAZA(set)) 
		cout << "Set " << set << " GetVictimInSet para addr " << std::hex << paddr << std::dec 
			 << " bloque " << std::hex << block << std::dec << " en cpu " << cpu << " instr " << get_instr_count(cpu) 
			 << " ciclo " << get_cycle_count() << " simulando " << (int)simulando << endl;
    
	if (BYPASS && (type != WRITEBACK))
		bypass_posible = 1;

	// En la primera vuelta usamos RANDOM, TODO_FALLOS o SRRIP
	if (simulando == 0)
	{
#ifdef SIM0_RANDOM

		return get_cycle_count() % LLC_WAYS;
		
#elif defined SIM0_TODO_FALLOS

		uint32_t via = 666;
		if (bypass_posible)
			via = 16;
		else
			via = 0;

		return via;
#else
	   // SRRIP: look for the maxRRPV line
		while (1)
		{
			for (int i=0; i<LLC_WAYS; i++)
				if (rrpv[set][i] == maxRRPV)
					return i;

			for (int i=0; i<LLC_WAYS; i++)
				rrpv[set][i]++;
		}
#endif		
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
	long long int offset_max=-1;
	
	for (uint32_t core=0; core<NUM_CORE; core++)
	{
		int pos_max_core=-1;

		// Hay que inicializar ahora y luego ir sumando los offset relativos
		long long int offset_it = OPT_accesses[core].sets[set].previdx_access_cycle;

		for (uint32_t i=OPT_accesses[core].sets[set].idx, l=0; i<OPT_accesses[core].sets[set].max_idx && num_por_encontrar[core]>0 && l < MAX_BUSQUEDA; i++, l++)
		{
			offset_it += ((long long int)OPT_accesses[core].sets[set].cycle_distance_previdx[i])*(100+factor_core[core])/100;

		   for (uint32_t j=0; j<LLC_WAYS+bypass_posible; j++)
			{
				if (   ((tmpOPTset[j].acceso_encontrado == false) && (tmpOPTset[j].cpu == core) && (tmpOPTset[j].bloque == OPT_accesses[core].sets[set].bloque[i]))
					&& !((j==LLC_WAYS) && (core==cpu) && (i==OPT_accesses[core].sets[set].idx)))
				{
					tmpOPTset[j].acceso_encontrado = true;
					num_por_encontrar[core]--;
					pos_max_core = j;
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
		if ((pos_max_core > -1) && (offset_it > offset_max))
		{
			offset_max = offset_it;
			pos_max=pos_max_core;
		}
	}
	
	if (TRAZA(set)) {
		cout << "Set " << set << ": Echamos bloque mas tarde a acceder en pos " << pos_max << endl;
	}

    return pos_max;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
	uint64_t ic = get_instr_count(cpu);
	uint64_t cc = get_cycle_count();

	int iHit = 0;
	if (hit)
		iHit = 1;

	uint64_t block = paddr >> 6; // assuming 64B line size, get rid of lower bits
	
	if (TRAZA(set)) 
		cout << "Set " << set << " UpdateReplacementState way " << way << " addr " << std::hex << paddr << " block " << block  << std::dec
			 << " type " << type << " hit " << iHit << " en cpu " << cpu << " instr " << ic << endl;
	
	if (type == WRITEBACK)
		return;

	// Buscamos el final del warmup. Cada traza esta en un numero de instruccion diferente
	if (!warmup_finished)
	{
		warmup_finished = true;
		for (uint32_t i=0; i<NUM_CORE; i++)
		{
			if (get_instr_count(i) < warmup_inst)
				warmup_finished = false;
		}
		if (warmup_finished)
		{
			for (uint32_t i=0; i<NUM_CORE; i++)
			{
				last_warmup_inst[i]=get_instr_count(i);
				cout << "Fin del warmup, en core " << i << " en la instruccion " << last_warmup_inst[i] << endl;
			}
		}
	}

	// Buscamos el final de la simulación con métrica, excluyendo lo extra. Cada traza esta en un numero de instruccion diferente
	if (warmup_finished && !measure_finished)
	{
		measure_finished = true;
		for (uint32_t i=0; i<NUM_CORE; i++)
		{
			if (get_instr_count(i) < last_warmup_inst[i] + simulation_inst)
				measure_finished = false;
		}
	}
		
	// Guardamos los bloques. 
	OPT_accesses_sig[cpu].sets[set].bloque.push_back(block);
	OPT_accesses_sig[cpu].sets[set].instr_count.push_back(ic);

	// Guardamos la distancia en ciclos entre este acceso y el anterior
	if (TRAZA(set)) 
		cout << "Set " << set << ": Meto acceso bloque " << std::hex << block << std::dec << " a cpu " << cpu << " idx " << OPT_accesses_sig[cpu].sets[set].idx  
			<< " distancia ciclos " << (uint32_t)(cc-OPT_accesses_sig[cpu].sets[set].previdx_access_cycle) << endl;

	OPT_accesses_sig[cpu].sets[set].cycle_distance_previdx.push_back((uint32_t)(cc-OPT_accesses_sig[cpu].sets[set].previdx_access_cycle));
	OPT_accesses_sig[cpu].sets[set].previdx_access_cycle = cc;
	OPT_accesses_sig[cpu].sets[set].idx++;		

	if (simulando > 0)
	{
		// Cuando ya tenemos la traza porque la hemos leido, vamos avanzando los indices de cada set con cada acceso
		if (OPT_accesses[cpu].sets[set].idx >= OPT_accesses[cpu].sets[set].max_idx) {
			if (measure_finished) {
				missing_extra++;
			} else {
				missing_measure++;
			}
		} else {
			while (OPT_accesses[cpu].sets[set].idx < OPT_accesses[cpu].sets[set].max_idx)
			{
				if (OPT_accesses[cpu].sets[set].bloque[OPT_accesses[cpu].sets[set].idx] == block)
				{
					if (TRAZA(set)) 
						cout << "Set " << set << ": Acceso esta en cpu " << cpu << " idx " << OPT_accesses[cpu].sets[set].idx  << endl;
			
					found++;
					OPT_accesses[cpu].sets[set].idx++;
					break;
				}
				else if (OPT_accesses[cpu].sets[set].instr_count[OPT_accesses[cpu].sets[set].idx] > ic)
				{
					if (TRAZA(set)) 
						cout << "Set " << set << ": Llego en cpu " << cpu << " a instr " << OPT_accesses[cpu].sets[set].instr_count[OPT_accesses[cpu].sets[set].idx] << " y no lo he encontrado "  << endl;
					notfound++;
					break;
				}
				else
				{
					if (TRAZA(set)) 
						cout << "Set " << set << ": Me salto acceso en cpu " << cpu << " idx " << OPT_accesses[cpu].sets[set].idx << endl;
					
					skip++;
					OPT_accesses[cpu].sets[set].idx++;
				}
			}
		}
	}

#ifdef SIM0_SRRIP	
	// En la primera iteracion se usa reemplazo SRRIP
	if (simulando == 0)
	{
		if (hit)
			rrpv[set][way] = 0;
		else
			rrpv[set][way] = maxRRPV-1;
	}
#endif

	return;
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{
	if (simulando > 0) {
		cout << "Found " << found << ", Not Found " << notfound << ", Skip " << skip 
			<< " missing_measure " << missing_measure << ", missing_extra " << missing_extra << endl;
	}

	if (   getenv("FICH_OPT_ACCESSES_SIG") == NULL
	    || boost::algorithm::contains(getenv("FICH_OPT_ACCESSES_SIG"),"/void"))
	{
		cout << "No se crea nuevo fichero de volcado de OPT_accesses_sig[]" << endl;
		return;
	}

	// Volcamos a fichero OPT_accesses
	
	for (uint32_t cpu=0; cpu < NUM_CORE; cpu++)
	{
		for (uint32_t i=0; i<LLC_SETS; i++) 
		{
			OPT_accesses_sig[cpu].sets[i].max_idx = OPT_accesses_sig[cpu].sets[i].idx;
		}
	}

	cout << "Comienzo guardado de OPT_accesses_sig[] fich " << getenv("FICH_OPT_ACCESSES_SIG") << " a " 
		<< std::ctime(new time_t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))) << endl;
	
	// create/overwrite and open a binary archive for output
	std::ofstream ofs(getenv("FICH_OPT_ACCESSES_SIG"), std::ios::binary | std::ios::trunc);
	boost::archive::binary_oarchive oa(ofs);
	
	// write class instance to archive
	for (uint32_t cpu=0; cpu<NUM_CORE; cpu++)
	{
		oa << OPT_accesses_sig[cpu];		
	}

	ofs.close();

	cout << "Fichero de volcado de OPT_accesses_sig[] creado a " 
		<< std::ctime(new time_t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))) << endl;
}

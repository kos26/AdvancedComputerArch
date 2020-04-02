#include "Branch_Predictor.h"

const unsigned instShiftAmt = 2; // Number of bits to shift a PC by

// You can play around with these settings.
const unsigned localPredictorSize = 2048;
const unsigned localCounterBits = 2;
const unsigned localHistoryTableSize = 2048; 
const unsigned globalPredictorSize = 65536;
const unsigned globalCounterBits = 2;
const unsigned choicePredictorSize = 8192; // Keep this the same as globalPredictorSize.
const unsigned choiceCounterBits = 2;

Branch_Predictor *initBranchPredictor()
{
    Branch_Predictor *branch_predictor = (Branch_Predictor *)malloc(sizeof(Branch_Predictor));



    #ifdef TOURNAMENT

    assert(checkPowerofTwo(globalPredictorSize));



    branch_predictor->global_predictor_size = globalPredictorSize;

   


    int i = 0;


    // Initialize global counters
    branch_predictor->global_counters = 
        (Sat_Counter *)malloc(globalPredictorSize * sizeof(Sat_Counter));

    //FOR PERCEPTRON
    branch_predictor->p_mesa = 
        (P_Table *)malloc(globalPredictorSize * sizeof(P_Table));

    for (i = 0; i < globalPredictorSize; i++)
    {
        initSatCounter(&(branch_predictor->global_counters[i]), 
globalCounterBits);

	//FOR PERCEPTRON
	initPTable(&(branch_predictor->p_mesa[i]));

    }

    branch_predictor->global_history_mask = globalPredictorSize - 1;


    #endif

    return branch_predictor;
}

// sat counter functions
inline void initSatCounter(Sat_Counter *sat_counter, unsigned counter_bits)
{
    sat_counter->counter_bits = counter_bits;
    sat_counter->counter = 0;
    sat_counter->max_val = (1 << counter_bits) - 1;
}

//FOR PERCEPTRON
void initPTable(P_Table *p_mesa)
{

    for (int i = 0; i < PLENGTH; i++)
    {
	p_mesa->weights[i] = (rand() % 21) - 11;
	//printf("WEIGHTS %d\n",p_mesa->weights[i]);
    }

}

inline void incrementCounter(Sat_Counter *sat_counter)
{
    if (sat_counter->counter < sat_counter->max_val)
    {
        ++sat_counter->counter;
    }
}

inline void decrementCounter(Sat_Counter *sat_counter)
{
    if (sat_counter->counter > 0) 
    {
        --sat_counter->counter;
    }
}

//FOR PERCEPTRON
void storeNewWgts(P_Table *p_mesa, int wgts[PLENGTH] )
{
    for (int i = 0; i<PLENGTH; i++)
    {
        p_mesa->weights[i] = wgts[i];
    }
}

// Branch Predictor functions
bool predict(Branch_Predictor *branch_predictor, Instruction *instr)
{
    uint64_t branch_address = instr->PC;


    #ifdef TOURNAMENT


    // Step two, get global prediction.
    unsigned global_predictor_idx = 
        (branch_predictor->global_history ^ branch_address) & branch_predictor->global_history_mask;



    //FOR PERCEPTRON

    int wgts[PLENGTH];
    uint64_t hist = branch_predictor->global_history;
    int prev[PLENGTH] = { 1 };
    int sum = 0;

    for (int i = 0; i < PLENGTH; i++)
    {
	wgts[i] = getWeights(&(branch_predictor->p_mesa[global_predictor_idx]))[i];

	if (i != 0)
	{
	    bool bit = (hist & 0x01) != 0;
            hist >>= 1;

	    if (bit)
	    {
	        prev[i] = 1;	
	    }else
    	    {
	        prev[i] = -1;
	    }
	    sum += prev[i] * wgts[i];
	}else{
	    sum += 1 * wgts[i];

	}
	
		

    }


    bool global_prediction;
    if (sum < 0)
    {
        global_prediction = false;
    }else{
	global_prediction = true;
    }





    // Step four, final prediction.
    bool final_prediction = global_prediction;

    bool prediction_correct = final_prediction == instr->taken;

    int t;
    if (instr->taken){
	t = 1;
    }else{
	t = -1;
    }

    //TRAIN PERCEPTRON HERE
    if (!prediction_correct | sum == 0){
        for (int i = 0; i < PLENGTH; i++)
        {
	    int change = t*prev[i];	

	    wgts[i] += change;


	    
	    storeNewWgts(&(branch_predictor->p_mesa[global_predictor_idx]), wgts );



        }
    }









    // Step five, update counters

    if (instr->taken)
    {
        incrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));

    }
    else
    {
        decrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));

    }

    // Step six, update global history register
    branch_predictor->global_history = branch_predictor->global_history << 1 | instr->taken;

    return prediction_correct;
    #endif
}

inline unsigned getIndex(uint64_t branch_addr, unsigned index_mask)
{
    return (branch_addr >> instShiftAmt) & index_mask;
}

inline bool getPrediction(Sat_Counter *sat_counter)
{
    uint8_t counter = sat_counter->counter;
    unsigned counter_bits = sat_counter->counter_bits;

    // MSB determins the direction
    return (counter >> (counter_bits - 1));
}

//FOR PERCEPTRON
int * getWeights( P_Table *p_table )
{
    int * wgts = p_table->weights;
    //for (int i = 0; i<33; i++)
    //	printf("I %d WEIGHTS %d \n",i, wgts[i]);
    return wgts;
}

int checkPowerofTwo(unsigned x)
{
    //checks whether a number is zero or not
    if (x == 0)
    {
        return 0;
    }

    //true till x is not equal to 1
    while( x != 1)
    {
        //checks whether a number is divisible by 2
        if(x % 2 != 0)
        {
            return 0;
        }
        x /= 2;
    }
    return 1;
}

#include "../HG.h"
const int M = 6e3, T = 16, D = 8;
const int QK = 500;
#define dataset 1
#if dataset==1
	const int TRACE_LEN = 16;
	const int TUPLE_LEN = 4;
#elif dataset==2
	const int TRACE_LEN = 26;
	const int TUPLE_LEN = 4;
#elif dataset==3
	const int TRACE_LEN = 4;
	const int TUPLE_LEN = 4;
	const int alpha = 30;
#endif
const int WIN = 1e6;
const int MAXINPUT = 1e7;
const int NUM = 3, LIM = 500;
uint32_t addr[MAXINPUT];
uint64_t timestamp[MAXINPUT];
pil strrepA[MAXINPUT], strrepB[MAXINPUT];
map<uint32_t, int> strmapA, strmapB;

uint64_t calc(uint32_t a, uint32_t b) {
	return 1ll * a * 1000000 + b;
}

void load_CAIDA() {
	char CAIDA[100];
	sprintf(CAIDA, "E:\\DataSet\\CAIDA\\formatted00.dat");
	ifstream fin(CAIDA, ios::binary);
    uint8_t key[TRACE_LEN];
    rep2 (pkt, 0, MAXINPUT) {
        fin.read((char *)key, TRACE_LEN);
        memcpy(addr+pkt, key+8, TUPLE_LEN);
//		timestamp[pkt] = calc(*((uint32_t *) key), *((uint32_t *) key+1));
		timestamp[pkt] = pkt;
    }
}
void load_IMC() {
	
	BOBHash32 * hash = new BOBHash32(uint8_t(rd() % MAX_PRIME32));
	char IMC[100];
	sprintf(IMC, "D:\\University\\Research\\yt\\sliding_window_topk\\data\\20.dat");
	ifstream fin(IMC, ios::binary);
    uint8_t key[TRACE_LEN];
    rep2 (pkt, 0, MAXINPUT) {
        fin.read((char *)key, TRACE_LEN);
        addr[pkt]= hash->run((char *) key, 13);
		timestamp[pkt] = pkt;
    }
}


void load_zipf(int alpha) {
	char zipf[100];
	sprintf(zipf, "E:\\DataSet\\new_zipf\\%03d.dat", alpha);
	ifstream fin(zipf, ios::binary);
    uint8_t key[TRACE_LEN];
    rep2 (pkt, 0, MAXINPUT) {
        fin.read((char *)key, TRACE_LEN);
        memcpy(addr+pkt, key, TUPLE_LEN);
		timestamp[pkt] = pkt;
    }
}

int main() {
#if dataset==1
    load_CAIDA();
#elif dataset==2
    load_IMC();
#elif dataset==3
	load_zipf(alpha);
#endif
	freopen("speed.csv", "w", stdout);
	srand(time(0));
	cout << "HG" << endl;
	for (int mem = 10; mem <= 320; mem <<= 1) {
		double mops = 0;
		for (int test = 1; test <= 20; test++) {
			strmapA.clear();
			strmapB.clear();
			int M = mem * 1024 * 8 / (32 + (T+2)*32);
			auto sketch = new HG<TUPLE_LEN>(M, WIN, T, D);
			double totare = 0, totrecall = 0;
			int cnt = 0, hA = 0, tA = 0, hB = 0, tB = 0, pkt = 0;

			clock_t st = clock();

			while (pkt < NUM * WIN) {
				sketch->insert((uint8_t *)(addr + pkt), timestamp[pkt]);
				pkt++;
			}

			clock_t ed = clock();
			
			mops += 1. / (1. * (ed - st) / CLOCKS_PER_SEC / (NUM * WIN)) / 1e6;
				
		}
		cout << mops / 20 << endl;
	
	}
    return 0;
}
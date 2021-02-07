#include "../MicroSketch-HG.h"
const int M = 6e3, T = 12, D = 8, size_k = 8, log_base = 1;
const int QK = 500;
const int TRACE_LEN = 16;
const int TUPLE_LEN = 4;
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
//		timestamp[pkt] = calc(*((uint32_t *) key), *((uint32_t *) key+1)); // time-based
		timestamp[pkt] = pkt; // count-based
    }
}

int main() {
    load_CAIDA();
	freopen("speed.csv", "w", stdout);
	srand(time(0));
	cout << "MicroSketch-HG" << endl;
	for (int mem = 10; mem <= 320; mem <<= 1) {
		double mops = 0;
		for (int test = 1; test <= 40; test++) {
			strmapA.clear();
			strmapB.clear();
			int M = mem * 1024 * 8 / (32 + (2*T+2)*size_k + 32-size_k + ceil(5 - log(log_base)/log(2)));
			auto sketch = new MicroSketch_HG<TUPLE_LEN>(M, WIN, T, D, size_k, log_base);
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
		cout << mops / 40 << endl;
	
	}
    return 0;
}
#include <bits/stdc++.h>
#include "BOBHash32.h"
using namespace std;
#define fi first
#define se second
#define mp make_pair
#define pb push_back
#define SZ(x) (int((x).size()))
#define All(x) (x).begin(), (x).end()
typedef pair<uint32_t, int> pii;
typedef pair<uint32_t, uint64_t> pil;
#define rep(i,a,b) for(int (i)=(a);(i)<=(b);++(i))
#define rep2(i,a,b) for(int (i)=(a);(i)<(b);++(i))

bool cmp(const pii & A, const pii & B) {
    return A.se > B.se;
}

long long rd() {
    return 1ll*rand()*rand()+rand();
}

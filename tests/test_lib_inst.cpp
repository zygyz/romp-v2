#include <iostream>
#include <math.h>
#include <omp.h>
#include <vector>
using namespace std;

int main(int argc, const char* argv[]) {
  vector<int> v = {1, 2, 3, 4, 5};
  #pragma omp parallel
  {
    auto num_threads = omp_get_num_threads();
    cout << "num thread: " << num_threads << endl;
    #pragma omp parallel for 
    for (int i = 0; i < 100; ++i) {
      v[i%5] += omp_get_thread_num();
      cout << "thread id: " << omp_get_thread_num() << endl;
      double dValue = sqrt(v[i%5]);
      cout << dValue << endl;
    }
    v[0]++;
     cout << "hello world\n";
  }
  return 0;
}

template<class T>
struct MyTestVector2
{
  T  _data[6];
  T * data() { return &_data[0]; }
};

struct TestClass
{
  void kernel_Init(unsigned int* flags, unsigned int tid);      
  
  void kernel_TestColor(const unsigned int* flags, 
                        unsigned int* out_color, unsigned int tid);

  void MainFunc(unsigned int* out_color, unsigned int tid);
};

void TestClass::kernel_Init(unsigned int* flags, unsigned int tid)
{
  flags[tid] = 1;
}      
  
void TestClass::kernel_TestColor(const unsigned int* flags, 
                                 unsigned int* out_color, unsigned int tid)
{
  if(flags[tid] == 1)
    out_color[tid] = 0x00000000;
  else  
    out_color[tid] = 0xFFFFFFFF;
}

void TestClass::MainFunc(unsigned int* out_color, unsigned int tid)
{
  unsigned int flags = 0;
  kernel_Init(&flags, tid);
  
  MyTestVector2<unsigned int> test2;
  kernel_TestColor(&flags, test2.data(), tid);
  //kernel_TestColor(&flags, out_color, tid);
}
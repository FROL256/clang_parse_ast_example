struct TestClass
{
  void kernel_TestColor(const unsigned int* flags, 
                        [[kslicer::size(a_colorSize)]] unsigned int* out_color, 
                        unsigned int a_colorSize);
};

void TestClass::kernel_TestColor(const unsigned int* flags, 
                                 unsigned int* out_color, 
                                 unsigned int a_colorSize)
{

}

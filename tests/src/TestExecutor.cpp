// #include "libserver/Executor.hpp"
//
// #include <cassert>
//
// namespace {
//
// void TestExecutor()
// {
//   const auto executor = new server::Executor(50);
//   int integer = 0;
//
//   for (int i = 0; i < 10000; i++)
//   {
//     executor->execute([&integer]() { integer++; });
//   }
//
//   delete executor;
//
//   assert(integer == 10000);
// }
//
// } // namespace anon
//
// int main() {
//   TestExecutor();
// }
//

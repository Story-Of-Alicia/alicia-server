// #include "libserver/Executor.hpp"
//
// namespace {
//
// void TestExecutorTimeout()
// {
//   const auto executor = new server::Executor(50);
//   executor->execute([]{
//     std::this_thread::sleep_for(std::chrono::milliseconds(500));
//   });
//   delete executor;
// }
//
// } // namespace anon
//
// int main() {
//   TestExecutorTimeout();
// }
//

- シンプル
  - [スレッド](https://github.com/DYGV/thread_process/blob/master/simple/thread_lock.c)
  - [プロセス (System V IPC)](https://github.com/DYGV/thread_process/blob/master/simple/process_lock.c)
  - [プロセス (POSIX IPC)](https://github.com/DYGV/thread_process/blob/master/simple/process_lock_posix_ver.c)

- [生産者・消費者問題](https://github.com/DYGV/thread_process/blob/master/producer_consumer_problem/pcp.c)
  - [スレッド](https://github.com/DYGV/thread_process/blob/master/producer_consumer_problem/thread.c)
  - [プロセス](https://github.com/DYGV/thread_process/blob/master/producer_consumer_problem/process.c)

- [哲学者の食事問題](https://github.com/DYGV/thread_process/blob/master/dining_philosophers_problem/dpp.c)
  - [スレッド](https://github.com/DYGV/thread_process/blob/master/dining_philosophers_problem/thread.c)
  - [プロセス](https://github.com/DYGV/thread_process/blob/master/dining_philosophers_problem/process.c)  


※ 大量の標準出力(1)がされるので、計測時は/dev/nullに捨てること  
例: `$ time ./dpp_thread > /dev/null`

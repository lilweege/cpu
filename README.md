# cpu
A 32-bit RISC-V emulator

![screenshot](https://github.com/lilweege/cpu/assets/59585724/234ace56-e7f2-4e93-a2f2-8279b0193642)

### TODO
- [x] Implement rv32i and Zicsr
- [x] Implement rv32m extension
- [x] Implement rv32f extension
- [ ] Implement rv32c extension
- [ ] Implement other CSRs and privilege modes
- [ ] Add buttons
    - [x] Open file
    - [x] Start
    - [ ] Stop
    - [x] Restart
    - [ ] Step over
    - [x] Step into
    - [ ] Step out
- [ ] Fix rv32f (some operations are currently implementation defined, works on msvc and gcc but not clang)
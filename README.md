# ISH

standard UNIX-based shell, includes all basic functionality of a shell

- **PROJECT STILL IN DEVELOPMENT**

## Feature Plans

- Redirection (done)
- Piping (done (mostly))
- Script automation (maybe minimal)

## Requirements

- **CMake** : minimum version required : 3.16
- **A C++ compiler** : g++ or clang++ (with C++20 support)
- **readline** : libreadline-dev on Debian/Ubuntu, readline on Arch/Fedora
- **make**

## Getting Started

- **Clone the repo**

```bash
git clone <https://github.com/Indective/ish.git>
cd ish
```

- **Configure and build**

```bash
cmake ..
```

- **Run**

```bash
cd build
make run
```

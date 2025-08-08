# X3MP Lint and Security Testing

This repository now includes comprehensive linting and security testing for both the Go server and C++ client components.

## Overview

The linting and security testing setup includes:

### Go Server (GoServer/)
- **golangci-lint**: Comprehensive Go linting with multiple analyzers
- **gosec**: Go security analyzer for vulnerability detection
- **go vet**: Built-in static analysis tool
- **gofmt/goimports**: Code formatting tools

### C++ Client (Client/)
- **cppcheck**: Static analysis for C++ code
- **clang-tidy**: Additional C++ linting and modernization checks
- **Security pattern analysis**: Detection of unsafe functions and potential vulnerabilities

## Quick Start

### Go Server

```bash
cd GoServer
make help                    # Show available commands
make install-tools          # Install linting and security tools
make check                  # Run all quality checks (format, vet, lint, security, test)
```

Individual commands:
```bash
make fmt                    # Format code
make vet                    # Run go vet
make lint                   # Run golangci-lint
make security               # Run gosec security scanner
make test                   # Run tests
make build                  # Build the server
```

### C++ Client

#### Linux/macOS
```bash
cd Client
./lint-and-security.sh     # Run all analyses
./lint-and-security.sh --help    # Show options
```

#### Windows (PowerShell)
```powershell
cd Client
.\lint-and-security.ps1    # Run all analyses
.\lint-and-security.ps1 -Help    # Show options
```

## Tool Installation

### Go Tools (Automatic)
The Makefile will automatically install required tools:
- golangci-lint
- gosec  
- goimports

### C++ Tools (Manual Installation Required)

#### Ubuntu/Debian
```bash
sudo apt-get install cppcheck clang-tidy
```

#### macOS
```bash
brew install cppcheck llvm
```

#### Windows
- **cppcheck**: Download from [http://cppcheck.sourceforge.net/](http://cppcheck.sourceforge.net/) or `choco install cppcheck`
- **clang-tidy**: Install LLVM from [https://llvm.org/](https://llvm.org/) or `choco install llvm`

## Configuration Files

### Go Server Configuration
- `.golangci.yml`: golangci-lint configuration with security-focused rules
- `Makefile`: Build and analysis commands

### C++ Client Configuration  
- `cppcheck.xml`: cppcheck project configuration
- `lint-and-security.sh`: Linux/macOS analysis script
- `lint-and-security.ps1`: Windows PowerShell analysis script

## GitHub Actions CI/CD

The repository includes automated CI/CD workflows:

- **GoServer CI** (`.github/workflows/goserver-ci.yml`): 
  - Runs on Go code changes
  - Performs linting, security scanning, and testing
  - Uploads SARIF reports for GitHub Security tab
  - Generates coverage reports

- **Client CI** (`.github/workflows/client-ci.yml`):
  - Runs on C++ code changes  
  - Performs static analysis on both Linux and Windows
  - Checks for security vulnerabilities
  - Uploads analysis reports as artifacts

## Reports and Output

### Go Server Reports
- `security-report.json`: gosec security findings
- `coverage.out/coverage.html`: Test coverage reports
- Console output from golangci-lint

### C++ Client Reports
- `reports/cppcheck-report.xml`: cppcheck analysis results
- `reports/security-report.xml`: Security-focused cppcheck results
- `reports/clang-tidy-report.txt`: clang-tidy findings
- `reports/security-patterns.txt`: Security pattern analysis
- `reports/cppcheck-html/`: HTML report (if cppcheck-htmlreport is available)

## Security Checks

### Go Security (gosec)
- Hardcoded credentials detection
- SQL injection vulnerabilities
- Unsafe cryptographic practices
- File system security issues
- Network security problems
- Integer overflow detection

### C++ Security
- Buffer overflow vulnerabilities
- Use of unsafe functions (strcpy, gets, etc.)
- Memory management issues
- Integer overflow potential
- Format string vulnerabilities

## Integration with IDEs

### VS Code
Install the following extensions for real-time linting:
- **Go**: Official Go extension with built-in linting
- **C/C++**: Microsoft C++ extension with IntelliSense
- **clangd**: Alternative C++ language server

### Visual Studio
The C++ project includes cppcheck integration through the XML configuration file.

## Best Practices

1. **Run checks before committing**:
   ```bash
   # Go server
   cd GoServer && make check
   
   # C++ client  
   cd Client && ./lint-and-security.sh
   ```

2. **Fix issues incrementally**: Address linting and security issues as they're found rather than accumulating technical debt.

3. **Review security warnings carefully**: Not all warnings are actionable, but security-related findings should be investigated.

4. **Use CI/CD feedback**: The GitHub Actions workflows provide automated feedback on pull requests.

## Troubleshooting

### Common Issues

1. **Tools not found**: Ensure tools are installed and in PATH
2. **False positives**: Use configuration files to suppress known false positives
3. **Windows path issues**: Use PowerShell script instead of bash script on Windows
4. **Missing dependencies**: Install build dependencies for C++ analysis

### Getting Help

- Check tool documentation: `golangci-lint help`, `cppcheck --help`
- Review configuration files for customization options
- Consult GitHub Actions logs for CI/CD issues

## Customization

### Modifying Go Linting Rules
Edit `.golangci.yml` to enable/disable specific linters or rules.

### Modifying C++ Analysis
Edit `cppcheck.xml` for cppcheck configuration, or modify the shell scripts for additional analysis tools.

### Adding New Tools
Both the Makefile and shell scripts can be extended to include additional analysis tools as needed.
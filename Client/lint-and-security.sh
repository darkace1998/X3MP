#!/bin/bash
# X3MP Client C++ Lint and Security Analysis Script
# This script runs static analysis tools on the C++ client code

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if cppcheck is installed
check_cppcheck() {
    if command -v cppcheck >/dev/null 2>&1; then
        print_success "cppcheck found: $(cppcheck --version)"
        return 0
    else
        print_error "cppcheck not found. Please install cppcheck."
        print_status "On Ubuntu/Debian: sudo apt-get install cppcheck"
        print_status "On macOS: brew install cppcheck"
        print_status "On Windows: Download from http://cppcheck.sourceforge.net/"
        return 1
    fi
}

# Check if clang-tidy is installed
check_clang_tidy() {
    if command -v clang-tidy >/dev/null 2>&1; then
        print_success "clang-tidy found: $(clang-tidy --version | head -n1)"
        return 0
    else
        print_warning "clang-tidy not found. Install for additional checks."
        print_status "On Ubuntu/Debian: sudo apt-get install clang-tidy"
        print_status "On macOS: brew install llvm"
        print_status "On Windows: Install LLVM from https://llvm.org/"
        return 1
    fi
}

# Run cppcheck analysis
run_cppcheck() {
    print_status "Running cppcheck static analysis..."
    
    # Create output directory
    mkdir -p reports
    
    # Run cppcheck with comprehensive checks
    cppcheck \
        --project=cppcheck.xml \
        --enable=all \
        --inconclusive \
        --force \
        --verbose \
        --template='{file}:{line}:{column}: {severity}: {message} [{id}]' \
        --xml \
        --xml-version=2 \
        --output-file=reports/cppcheck-report.xml \
        2>&1 | tee reports/cppcheck-output.txt
    
    # Generate HTML report if possible
    if command -v cppcheck-htmlreport >/dev/null 2>&1; then
        print_status "Generating HTML report..."
        cppcheck-htmlreport \
            --file=reports/cppcheck-report.xml \
            --report-dir=reports/cppcheck-html \
            --source-dir=.
        print_success "HTML report generated in reports/cppcheck-html/"
    fi
    
    # Check for errors
    if grep -q "error" reports/cppcheck-output.txt; then
        print_error "cppcheck found errors in the code"
        return 1
    else
        print_success "cppcheck analysis completed successfully"
        return 0
    fi
}

# Run clang-tidy analysis (if available)
run_clang_tidy() {
    if ! command -v clang-tidy >/dev/null 2>&1; then
        print_warning "Skipping clang-tidy (not installed)"
        return 0
    fi
    
    print_status "Running clang-tidy analysis..."
    
    # Create a compilation database if it doesn't exist
    if [ ! -f compile_commands.json ]; then
        print_warning "No compile_commands.json found. Creating basic configuration..."
        cat > compile_commands.json << 'EOF'
[
    {
        "directory": ".",
        "command": "cl.exe /I. /DWIN32 /D_WINDOWS /D_USRDLL /D_WINDLL client.cpp",
        "file": "client.cpp"
    }
]
EOF
    fi
    
    # List of C++ source files
    CPP_FILES=(
        "client.cpp"
        "Chatbox.cpp"
        "Console.cpp"
        "Mod.cpp"
        "Renderer.cpp"
        "X3Functions.cpp"
        "X3Util.cpp"
        "directx.cpp"
        "mem.cpp"
        "settings.cpp"
        "dllmain.cpp"
    )
    
    # Run clang-tidy on each file
    mkdir -p reports
    echo "" > reports/clang-tidy-report.txt
    
    for file in "${CPP_FILES[@]}"; do
        if [ -f "$file" ]; then
            print_status "Analyzing $file with clang-tidy..."
            clang-tidy "$file" \
                --checks='-*,readability-*,performance-*,modernize-*,bugprone-*,clang-analyzer-*,cppcoreguidelines-*,hicpp-*,cert-*,misc-*' \
                --format-style=file \
                --header-filter='.*' \
                >> reports/clang-tidy-report.txt 2>&1 || true
        fi
    done
    
    print_success "clang-tidy analysis completed"
    return 0
}

# Run security-focused analysis
run_security_analysis() {
    print_status "Running security-focused analysis..."
    
    mkdir -p reports
    
    # Run cppcheck with security focus
    print_status "Running cppcheck with security checks..."
    cppcheck \
        --enable=warning,style,performance,portability,information \
        --inconclusive \
        --force \
        --template='{file}:{line}:{column}: {severity}: {message} [{id}]' \
        --xml \
        --xml-version=2 \
        --output-file=reports/security-report.xml \
        . 2>&1 | tee reports/security-output.txt
    
    # Check for common security issues
    print_status "Checking for common security patterns..."
    
    # Search for potentially unsafe functions
    UNSAFE_FUNCTIONS=(
        "strcpy"
        "strcat"
        "sprintf"
        "gets"
        "scanf"
        "strncpy"
        "strncat"
    )
    
    echo "=== Security Pattern Analysis ===" > reports/security-patterns.txt
    for func in "${UNSAFE_FUNCTIONS[@]}"; do
        if grep -rn "$func" --include="*.cpp" --include="*.h" . >> reports/security-patterns.txt 2>/dev/null; then
            print_warning "Found potentially unsafe function: $func"
        fi
    done
    
    # Check for buffer overflow patterns
    if grep -rn "char.*\[" --include="*.cpp" --include="*.h" . | grep -v "const" >> reports/security-patterns.txt; then
        print_warning "Found fixed-size buffers that may be vulnerable to overflow"
    fi
    
    print_success "Security analysis completed"
}

# Main function
main() {
    print_status "Starting C++ lint and security analysis for X3MP Client"
    
    # Check for required tools
    CPPCHECK_AVAILABLE=false
    CLANG_TIDY_AVAILABLE=false
    
    if check_cppcheck; then
        CPPCHECK_AVAILABLE=true
    fi
    
    if check_clang_tidy; then
        CLANG_TIDY_AVAILABLE=true
    fi
    
    if [ "$CPPCHECK_AVAILABLE" = false ]; then
        print_error "No static analysis tools available. Please install cppcheck at minimum."
        exit 1
    fi
    
    # Create reports directory
    mkdir -p reports
    
    # Run analyses
    EXIT_CODE=0
    
    if [ "$CPPCHECK_AVAILABLE" = true ]; then
        if ! run_cppcheck; then
            EXIT_CODE=1
        fi
        
        run_security_analysis
    fi
    
    if [ "$CLANG_TIDY_AVAILABLE" = true ]; then
        run_clang_tidy
    fi
    
    # Summary
    print_status "Analysis completed. Reports available in:"
    ls -la reports/ 2>/dev/null || true
    
    if [ $EXIT_CODE -eq 0 ]; then
        print_success "All analyses completed successfully"
    else
        print_error "Some analyses found issues. Check the reports for details."
    fi
    
    exit $EXIT_CODE
}

# Help function
show_help() {
    echo "X3MP Client C++ Lint and Security Analysis"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  --cppcheck     Run only cppcheck analysis"
    echo "  --clang-tidy   Run only clang-tidy analysis"
    echo "  --security     Run only security analysis"
    echo ""
    echo "This script runs static analysis tools on the C++ client code."
    echo "It requires cppcheck to be installed, and optionally clang-tidy."
}

# Parse command line arguments
case "${1:-}" in
    -h|--help)
        show_help
        exit 0
        ;;
    --cppcheck)
        check_cppcheck && run_cppcheck
        exit $?
        ;;
    --clang-tidy)
        check_clang_tidy && run_clang_tidy
        exit $?
        ;;
    --security)
        check_cppcheck && run_security_analysis
        exit $?
        ;;
    "")
        main
        ;;
    *)
        print_error "Unknown option: $1"
        show_help
        exit 1
        ;;
esac
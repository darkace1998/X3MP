# X3MP Client C++ Lint and Security Analysis Script (PowerShell)
# This script runs static analysis tools on the C++ client code

param(
    [switch]$CppCheck,
    [switch]$ClangTidy,
    [switch]$Security,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Colors for output
function Write-Status {
    param($Message)
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Write-Success {
    param($Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Warning {
    param($Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-Error {
    param($Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# Check if cppcheck is installed
function Test-CppCheck {
    try {
        $version = & cppcheck --version 2>$null
        Write-Success "cppcheck found: $version"
        return $true
    }
    catch {
        Write-Error "cppcheck not found. Please install cppcheck."
        Write-Status "Download from: http://cppcheck.sourceforge.net/"
        Write-Status "Or install via chocolatey: choco install cppcheck"
        return $false
    }
}

# Check if clang-tidy is installed
function Test-ClangTidy {
    try {
        $version = & clang-tidy --version 2>$null | Select-Object -First 1
        Write-Success "clang-tidy found: $version"
        return $true
    }
    catch {
        Write-Warning "clang-tidy not found. Install for additional checks."
        Write-Status "Install LLVM from: https://llvm.org/"
        Write-Status "Or install via chocolatey: choco install llvm"
        return $false
    }
}

# Run cppcheck analysis
function Invoke-CppCheck {
    Write-Status "Running cppcheck static analysis..."
    
    # Create output directory
    New-Item -ItemType Directory -Force -Path "reports" | Out-Null
    
    # Run cppcheck with comprehensive checks
    try {
        & cppcheck `
            --project=cppcheck.xml `
            --enable=all `
            --inconclusive `
            --force `
            --verbose `
            --template='{file}:{line}:{column}: {severity}: {message} [{id}]' `
            --xml `
            --xml-version=2 `
            --output-file=reports/cppcheck-report.xml `
            . 2>&1 | Tee-Object -FilePath "reports/cppcheck-output.txt"
        
        # Check for errors
        $output = Get-Content "reports/cppcheck-output.txt" -ErrorAction SilentlyContinue
        if ($output -match "error") {
            Write-Error "cppcheck found errors in the code"
            return $false
        }
        else {
            Write-Success "cppcheck analysis completed successfully"
            return $true
        }
    }
    catch {
        Write-Error "Failed to run cppcheck: $_"
        return $false
    }
}

# Run clang-tidy analysis (if available)
function Invoke-ClangTidy {
    if (-not (Test-ClangTidy)) {
        Write-Warning "Skipping clang-tidy (not installed)"
        return $true
    }
    
    Write-Status "Running clang-tidy analysis..."
    
    # Create a compilation database if it doesn't exist
    if (-not (Test-Path "compile_commands.json")) {
        Write-Warning "No compile_commands.json found. Creating basic configuration..."
        $compileDb = @'
[
    {
        "directory": ".",
        "command": "cl.exe /I. /DWIN32 /D_WINDOWS /D_USRDLL /D_WINDLL client.cpp",
        "file": "client.cpp"
    }
]
'@
        $compileDb | Out-File -FilePath "compile_commands.json" -Encoding UTF8
    }
    
    # List of C++ source files
    $cppFiles = @(
        "client.cpp",
        "Chatbox.cpp",
        "Console.cpp",
        "Mod.cpp",
        "Renderer.cpp",
        "X3Functions.cpp",
        "X3Util.cpp",
        "directx.cpp",
        "mem.cpp",
        "settings.cpp",
        "dllmain.cpp"
    )
    
    # Run clang-tidy on each file
    New-Item -ItemType Directory -Force -Path "reports" | Out-Null
    "" | Out-File -FilePath "reports/clang-tidy-report.txt"
    
    foreach ($file in $cppFiles) {
        if (Test-Path $file) {
            Write-Status "Analyzing $file with clang-tidy..."
            try {
                & clang-tidy $file `
                    --checks='-*,readability-*,performance-*,modernize-*,bugprone-*,clang-analyzer-*,cppcoreguidelines-*,hicpp-*,cert-*,misc-*' `
                    --format-style=file `
                    --header-filter='.*' `
                    2>&1 | Add-Content -Path "reports/clang-tidy-report.txt"
            }
            catch {
                # Continue on clang-tidy errors
                Write-Warning "clang-tidy had issues with $file"
            }
        }
    }
    
    Write-Success "clang-tidy analysis completed"
    return $true
}

# Run security-focused analysis
function Invoke-SecurityAnalysis {
    Write-Status "Running security-focused analysis..."
    
    New-Item -ItemType Directory -Force -Path "reports" | Out-Null
    
    # Run cppcheck with security focus
    Write-Status "Running cppcheck with security checks..."
    try {
        & cppcheck `
            --enable=warning,style,performance,portability,information `
            --inconclusive `
            --force `
            --template='{file}:{line}:{column}: {severity}: {message} [{id}]' `
            --xml `
            --xml-version=2 `
            --output-file=reports/security-report.xml `
            . 2>&1 | Tee-Object -FilePath "reports/security-output.txt"
    }
    catch {
        Write-Warning "Security cppcheck analysis had issues"
    }
    
    # Check for common security issues
    Write-Status "Checking for common security patterns..."
    
    # Search for potentially unsafe functions
    $unsafeFunctions = @(
        "strcpy",
        "strcat", 
        "sprintf",
        "gets",
        "scanf",
        "strncpy",
        "strncat"
    )
    
    "=== Security Pattern Analysis ===" | Out-File -FilePath "reports/security-patterns.txt"
    
    foreach ($func in $unsafeFunctions) {
        $matches = Select-String -Path "*.cpp", "*.h" -Pattern $func -ErrorAction SilentlyContinue
        if ($matches) {
            Write-Warning "Found potentially unsafe function: $func"
            $matches | Add-Content -Path "reports/security-patterns.txt"
        }
    }
    
    # Check for buffer overflow patterns
    $bufferMatches = Select-String -Path "*.cpp", "*.h" -Pattern "char.*\[" -ErrorAction SilentlyContinue | Where-Object { $_.Line -notmatch "const" }
    if ($bufferMatches) {
        Write-Warning "Found fixed-size buffers that may be vulnerable to overflow"
        $bufferMatches | Add-Content -Path "reports/security-patterns.txt"
    }
    
    Write-Success "Security analysis completed"
    return $true
}

# Main function
function Invoke-Main {
    Write-Status "Starting C++ lint and security analysis for X3MP Client"
    
    # Check for required tools
    $cppCheckAvailable = Test-CppCheck
    $clangTidyAvailable = Test-ClangTidy
    
    if (-not $cppCheckAvailable) {
        Write-Error "No static analysis tools available. Please install cppcheck at minimum."
        exit 1
    }
    
    # Create reports directory
    New-Item -ItemType Directory -Force -Path "reports" | Out-Null
    
    # Run analyses
    $exitCode = 0
    
    if ($cppCheckAvailable) {
        if (-not (Invoke-CppCheck)) {
            $exitCode = 1
        }
        
        Invoke-SecurityAnalysis | Out-Null
    }
    
    if ($clangTidyAvailable) {
        Invoke-ClangTidy | Out-Null
    }
    
    # Summary
    Write-Status "Analysis completed. Reports available in:"
    Get-ChildItem -Path "reports" -ErrorAction SilentlyContinue | Format-Table Name, Length, LastWriteTime
    
    if ($exitCode -eq 0) {
        Write-Success "All analyses completed successfully"
    }
    else {
        Write-Error "Some analyses found issues. Check the reports for details."
    }
    
    exit $exitCode
}

# Help function
function Show-Help {
    Write-Host "X3MP Client C++ Lint and Security Analysis"
    Write-Host ""
    Write-Host "Usage: .\lint-and-security.ps1 [OPTIONS]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Help          Show this help message"
    Write-Host "  -CppCheck      Run only cppcheck analysis"
    Write-Host "  -ClangTidy     Run only clang-tidy analysis"  
    Write-Host "  -Security      Run only security analysis"
    Write-Host ""
    Write-Host "This script runs static analysis tools on the C++ client code."
    Write-Host "It requires cppcheck to be installed, and optionally clang-tidy."
}

# Parse command line arguments
if ($Help) {
    Show-Help
    exit 0
}
elseif ($CppCheck) {
    if (Test-CppCheck) { Invoke-CppCheck }
}
elseif ($ClangTidy) {
    if (Test-ClangTidy) { Invoke-ClangTidy }
}
elseif ($Security) {
    if (Test-CppCheck) { Invoke-SecurityAnalysis }
}
else {
    Invoke-Main
}
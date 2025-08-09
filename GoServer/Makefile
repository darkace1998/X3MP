# X3MP GoServer Build and Lint Tools

# Add Go bin to PATH if not already there
export PATH := $(shell go env GOPATH)/bin:$(PATH)

.PHONY: help build test lint security fmt vet check clean install-tools

# Default target
help: ## Show this help message
	@echo "Available targets:"
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

##@ Build Commands

build: ## Build the GoServer binary
	@echo "Building GoServer..."
	go build -o bin/x3mp_goserver .

clean: ## Clean build artifacts
	@echo "Cleaning build artifacts..."
	rm -rf bin/
	go clean

##@ Testing Commands

test: ## Run all tests
	@echo "Running tests..."
	go test -v ./...

test-coverage: ## Run tests with coverage
	@echo "Running tests with coverage..."
	go test -v -coverprofile=coverage.out ./...
	go tool cover -html=coverage.out -o coverage.html
	@echo "Coverage report generated: coverage.html"

##@ Code Quality Commands

fmt: ## Format Go code
	@echo "Formatting Go code..."
	go fmt ./...
	goimports -w .

vet: ## Run go vet
	@echo "Running go vet..."
	go vet ./...

lint: install-tools ## Run golangci-lint
	@echo "Running golangci-lint..."
	golangci-lint run

security: install-tools ## Run security analysis with gosec
	@echo "Running security analysis..."
	gosec -fmt=json -out=security-report.json ./...
	@echo "Security report generated: security-report.json"
	gosec ./...

##@ Full Analysis

check: fmt vet lint security test ## Run all quality checks (format, vet, lint, security, test)

##@ Tool Installation

install-tools: ## Install required linting and security tools
	@echo "Installing golangci-lint..."
	@which golangci-lint > /dev/null || (echo "Installing golangci-lint..." && go install github.com/golangci/golangci-lint/cmd/golangci-lint@latest)
	@echo "Installing gosec..."
	@which gosec > /dev/null || (echo "Installing gosec..." && go install github.com/securego/gosec/v2/cmd/gosec@latest)
	@echo "Installing goimports..."
	@which goimports > /dev/null || (echo "Installing goimports..." && go install golang.org/x/tools/cmd/goimports@latest)

##@ Development

dev-setup: install-tools ## Set up development environment
	@echo "Setting up development environment..."
	go mod tidy
	go mod download

dev-check: ## Quick development check (fmt, vet, build, test)
	@echo "Running quick development check..."
	$(MAKE) fmt
	$(MAKE) vet
	$(MAKE) build
	$(MAKE) test

##@ CI/CD

ci: ## Run CI pipeline (all checks)
	@echo "Running CI pipeline..."
	$(MAKE) dev-setup
	$(MAKE) check

# Define variables for binary output
BIN_DIR := bin
BINARY := $(BIN_DIR)/x3mp_goserver

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build target that depends on bin directory
$(BINARY): $(BIN_DIR)
	go build -o $(BINARY) .
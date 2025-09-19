CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
DEBUG_FLAGS = -g -DDEBUG -O0
SANITIZER_FLAGS = -fsanitize=address -fsanitize=undefined

SRCDIR = src
INCDIR = include
TESTDIR = tests
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
HEADERS = $(wildcard $(INCDIR)/*.h)

MAIN_OBJ = $(OBJDIR)/main.o
LIB_OBJECTS = $(filter-out $(MAIN_OBJ), $(OBJECTS))

TARGET = kmp_demo
TEST_TARGET = test_kmp
BENCHMARK_TARGET = benchmark

.PHONY: all clean debug sanitize test benchmark install help

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

sanitize: CFLAGS += $(DEBUG_FLAGS) $(SANITIZER_FLAGS)
sanitize: clean $(TARGET) $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(LIB_OBJECTS) $(OBJDIR)/test_kmp.o | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/test_kmp.o: $(TESTDIR)/test_kmp.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

benchmark: $(BENCHMARK_TARGET)
	./$(BENCHMARK_TARGET)

$(BENCHMARK_TARGET): $(LIB_OBJECTS) $(OBJDIR)/benchmark.o | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/benchmark.o: $(TESTDIR)/benchmark.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

install: $(TARGET)
	@echo "Installing KMP demo to /usr/local/bin (requires sudo)"
	sudo cp $(TARGET) /usr/local/bin/

uninstall:
	@echo "Removing KMP demo from /usr/local/bin (requires sudo)"
	sudo rm -f /usr/local/bin/$(TARGET)

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) --demo

profile: CFLAGS += -pg
profile: clean $(TARGET)
	./$(TARGET) --demo
	gprof $(TARGET) gmon.out > profile.txt
	@echo "Profile report generated in profile.txt"

coverage: CFLAGS += --coverage
coverage: clean $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	gcov $(SOURCES)
	@echo "Coverage report generated"

docs:
	@echo "Generating documentation..."
	doxygen Doxyfile 2>/dev/null || echo "Doxygen not found, skipping documentation generation"

format:
	@echo "Formatting source code..."
	clang-format -i $(SOURCES) $(HEADERS) $(TESTDIR)/*.c 2>/dev/null || echo "clang-format not found"

lint:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c99 $(SRCDIR) $(INCDIR) 2>/dev/null || echo "cppcheck not found"

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET) $(TEST_TARGET) $(BENCHMARK_TARGET)
	rm -f *.gcov *.gcda *.gcno gmon.out profile.txt
	rm -f core vgcore.*

distclean: clean
	rm -rf docs/html docs/latex

check-syntax:
	$(CC) $(CFLAGS) -I$(INCDIR) -fsyntax-only $(SOURCES)

$(OBJDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -I$(INCDIR) -MM $< > $@.tmp
	@sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.tmp > $@
	@rm -f $@.tmp

help:
	@echo "Available targets:"
	@echo "  all         - Build the main demo program (default)"
	@echo "  debug       - Build with debug symbols and no optimization"
	@echo "  sanitize    - Build with AddressSanitizer and UBSan"
	@echo "  test        - Build and run unit tests"
	@echo "  benchmark   - Build and run performance benchmarks"
	@echo "  install     - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall   - Remove from /usr/local/bin (requires sudo)"
	@echo "  valgrind    - Run with Valgrind memory checker"
	@echo "  profile     - Build with profiling and generate profile report"
	@echo "  coverage    - Build with coverage and generate coverage report"
	@echo "  docs        - Generate documentation with Doxygen"
	@echo "  format      - Format source code with clang-format"
	@echo "  lint        - Run static analysis with cppcheck"
	@echo "  clean       - Remove build artifacts"
	@echo "  distclean   - Remove all generated files"
	@echo "  check-syntax- Check syntax without building"
	@echo "  help        - Show this help message"

ifndef NOMAKEFILEDEPS
-include $(OBJECTS:%.o=%.d)
endif
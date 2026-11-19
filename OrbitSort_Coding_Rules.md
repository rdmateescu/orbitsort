# OrbitSort Refactoring Rules

Use this note when continuing the OrbitSort code refactoring.

## Goal and scope

- This is a readability refactor, not an algorithm redesign.
- Preserve the original OrbitSort idea, circular pairing, and rotating adjacent-pair comparisons.
- Preserve the existing phase logic, stopping conditions, error codes, and output behavior.
- Keep the sequential and OpenMP implementations functionally equivalent.
- Do not add a new sorting strategy, architecture, or unnecessary abstractions.

## C style and control flow

- Follow the straightforward C style and formatting already used in the source. Do not introduce C++ features, clever expressions, or unnecessary helpers.
- Keep the existing validation and error-checking order.
- Store failures in `error_code`; only continue a step or loop while the error code is `ORBITSORT_SUCCESS`.
- Do not use early exits. On failure, stop further sorting or validation work, but still flow through the function's exit log and its single return at the end.
- Keep the code easy to follow. Reduce nesting only when it does not obscure the established step-by-step structure.

## KISS and SEC

- **KISS** means **Keep It Simple, Stupid**. Use direct, familiar C control flow. Avoid clever shortcuts, unnecessary abstractions, and changes that make the algorithm harder to explain.
- **SEC** means **Self-Explanatory Code**. Use clear names and explicit steps so the code communicates what it does. Comments should explain the algorithm and non-obvious decisions, and the complete source should be usable to present OrbitSort.
- Algorithm integrity
	- Do not change an agreed algorithm rule without discussing it first.
	- Do not introduce assumptions to resolve ambiguities. Identify them and ask for clarification.
	- Distinguish between an implementation detail and an algorithmic decision.
	- The code must implement the agreed algorithm, not silently redefine it for convenience.

## Logging and comments

- Keep each function's entry log near the beginning and its matching exit log immediately before the return.
- Pass a plain-language exit reason to every log_function_exit() call. State the successful stopping condition or the failure that stopped the function; do not use a generic message such as "Function exited."
- Preserve existing log levels, function names, and non-exit messages. Exit messages must explain the successful stopping condition or the failure reason.
- Cast intentionally ignored logging results to `(void)`, as in the source.
- Comments must be "stupid-proof": use plain, explicit language that a beginner can understand without relying on prior conversations or unstated context.
- Make the complete source readable as a presentation of the OrbitSort algorithm. Explain what each stage does, why it is needed, and how it relates to the algorithm.
- Preserve and improve the function documentation blocks and numbered `STEP` comments. Explain non-obvious details such as circular pair selection, wrap-around handling, phase updates, stopping conditions, error handling, and OpenMP synchronization where they occur.
- Keep comments accurate and aligned with the code. Do not remove useful explanation just to shorten the code.

## Refactoring workflow

- Preserve the original OrbitSort algorithm and behavior. Small interface changes are allowed when needed to share support functions with `orbitsort_refactored.c`.
- Put shared error codes, log-level types, and helper declarations in `orbitsort_internal.h`. Implement logging in `orbitsort_logging.c`. Do not include one `.c` file from another.
- Keep helpers private with `static` unless another source file must call them. Declare shared functions in the appropriate internal header.
- Put the six refactored functions in `orbitsort_refactored.c`.
- Work one function at a time: show the proposed revision, explain its changes, and wait for approval before moving to the next function.
- Finish the six-function refactor before reorganizing other code.
	Testing
		- Keep the original OrbitSort tests as regression tests.
		- Add tests for each new algorithmic rule before optimizing it.
		- Test sequential and OpenMP implementations independently.
		- Include boundary cases, duplicates, negative values, extreme integer values, and odd/even active-vector lengths.
		- Test memory allocation failures where practical.
		- Performance comparisons must use identical input data and conditions.
	Incremental development
		- Discuss the proposed design before implementation.
		- Work incrementally, one function at a time.
		- Explain each proposed change and its consequences.
		- Wait for approval before proceeding.
		- Do not optimize code until its correctness has been established.

##  Algorithm and implementation integrity
      Data integrity
	- Preserve all original values. No operation, including a merge, may lose, duplicate or alter a value.
	- Every operation that changes the structure of the active vector must preserve the correspondence between its elements and their original values.
	- Keep the distinction between the number of active elements and the total number of original values.
      Sequential and OpenMP equivalence
	- Sequential and OpenMP implementations must follow the same algorithmic rules and produce equivalent results.
	- A phase must finish completely before its results are applied and the next phase begins.
	- No thread may access an element through an index made invalid by a merge or compaction.
	- Shared data must have clearly defined ownership and synchronization.
	- Avoid unnecessary critical sections and synchronization, but never at the expense of correctness.

##  Memory management
	Allocation and memory safety
		- Every allocation must have a clearly defined owner and lifetime.
		- Check allocation failures and handle them through the established error-handling pattern.
		- Avoid unnecessary copying and memory movement.
		- Release all allocated memory on both success and failure paths.
		- Check size calculations for integer overflow before allocating memory.

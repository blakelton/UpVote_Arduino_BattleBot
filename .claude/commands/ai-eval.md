---
description: Deep codebase quality analysis with component-level and summary reports
---

# AI Evaluation Command

## Purpose

Perform comprehensive codebase quality analysis covering:
- DRY and SOLID principles
- Cyclomatic, cognitive, and McCabe complexity
- Race conditions and concurrency issues
- Design patterns and anti-patterns
- Performance considerations
- Foundational architecture review

Creates detailed reports for each major component and a prioritized summary report.

## Usage

```
/ai_eval
```

## Execution Steps

### Phase 1: Setup

#### 1.1 Create Output Directory

```bash
mkdir -p docs/ai_eval
```

If directory exists, proceed. Reports will be updated in place.

#### 1.2 Initialize Tracking

```markdown
## AI Evaluation Started

**Timestamp**: [YYYY-MM-DD HH:MM]
**Scope**: Full codebase analysis
```

### Phase 2: Component Discovery

#### 2.1 Scan Codebase Structure

Identify major components by analyzing:
- Top-level directories
- Module structure
- Package boundaries
- Logical groupings

```markdown
## Component Discovery

### Detected Components

| Component | Type | Location | Description |
|-----------|------|----------|-------------|
| [name] | [module/package/service] | [path] | [auto-detected purpose] |
```

#### 2.2 Categorize by Domain

Based on `project-config.yaml`, categorize components:

- **Embedded Components**: Firmware, drivers, HAL layers
- **Python Components**: Services, utilities, CLI tools
- **Web Components**: Frontend, API handlers, UI components
- **Shared/Core**: Cross-cutting concerns, utilities

#### 2.3 Create Component Registry

Save to `docs/ai_eval/_component_registry.md`:

```markdown
# Component Registry

**Generated**: [timestamp]
**Total Components**: [N]

## Components by Domain

### Embedded
- [component]: [path] - [brief description]

### Python
- [component]: [path] - [brief description]

### Web
- [component]: [path] - [brief description]

### Core/Shared
- [component]: [path] - [brief description]
```

### Phase 3: Component Analysis

For EACH major component, invoke the appropriate quality evaluator agent.

#### 3.1 Select Evaluator by Domain

| Component Type | Evaluator Agent |
|----------------|-----------------|
| Embedded/C/C++ | `embedded-quality-evaluator` |
| Python | `python-quality-evaluator` |
| Web/JS/TS | `web-quality-evaluator` |
| Mixed | Run multiple evaluators |

#### 3.2 Deep Analysis Protocol

For each component, the evaluator performs:

**A. DRY Analysis**
- Code duplication detection (exact and near-duplicate)
- Magic number/string identification
- Repeated patterns that should be extracted
- Copy-paste code detection

**B. SOLID Analysis**
- **S**ingle Responsibility violations
- **O**pen/Closed principle adherence
- **L**iskov Substitution compliance
- **I**nterface Segregation assessment
- **D**ependency Inversion evaluation

**C. Complexity Metrics**
- **Cyclomatic Complexity (CC)**: Branch/decision count
- **Cognitive Complexity**: Mental effort to understand
- **McCabe Complexity**: Extended cyclomatic analysis
- **Nesting Depth**: Maximum indentation levels
- **Function Length**: Lines per function/method

**D. Concurrency Analysis**
- Race condition detection
- Deadlock potential
- Shared state without protection
- Async/await correctness
- Thread safety issues

**E. Pattern Analysis**
- Design patterns used (correctly/incorrectly)
- Anti-patterns detected
- Idiomatic vs non-idiomatic code
- Framework pattern compliance

**F. Performance Review**
- Algorithmic complexity (Big-O)
- Unnecessary allocations
- N+1 query patterns
- Inefficient loops
- Resource usage patterns

**G. Error Handling**
- Exception handling completeness
- Error propagation correctness
- Silent failure detection
- Recovery mechanism assessment

#### 3.3 Generate Component Report

Create `docs/ai_eval/[component_name].md`:

```markdown
# Component Analysis: [Component Name]

**Generated**: [timestamp]
**Location**: [path]
**Domain**: [embedded/python/web]
**Lines of Code**: [N]
**Files Analyzed**: [N]

## Executive Summary

**Overall Grade**: [A|B|C|D|F]
**Critical Issues**: [N]
**High Issues**: [N]
**Technical Debt Score**: [Low|Medium|High|Critical]

[2-3 sentence summary of component health]

## Complexity Metrics

| Metric | Value | Threshold | Status |
|--------|-------|-----------|--------|
| Max Cyclomatic Complexity | [X] | 10 | [PASS/FAIL] |
| Max Cognitive Complexity | [X] | 15 | [PASS/FAIL] |
| Max Function Length | [X] | 50 | [PASS/FAIL] |
| Max Nesting Depth | [X] | 4 | [PASS/FAIL] |
| Average CC | [X] | 5 | [PASS/FAIL] |

### Complexity Hotspots

| Location | CC | Cognitive | Length | Action |
|----------|-----|-----------|--------|--------|
| [file:func] | [X] | [X] | [X] | [refactor/split/rewrite] |

## DRY Analysis

### Code Duplication

| Severity | Count | Description |
|----------|-------|-------------|
| CRITICAL | [N] | Exact duplicates > 20 lines |
| HIGH | [N] | Exact duplicates 10-20 lines |
| MEDIUM | [N] | Similar patterns 5-10 lines |

### Duplication Details

| Location 1 | Location 2 | Lines | Similarity |
|------------|------------|-------|------------|
| [file:line] | [file:line] | [N] | [X]% |

### Magic Values

| Location | Value | Suggested Constant |
|----------|-------|--------------------|
| [file:line] | [value] | [SUGGESTED_NAME] |

## SOLID Analysis

### Single Responsibility
**Status**: [PASS/WARN/FAIL]

| Location | Issue | Recommendation |
|----------|-------|----------------|
| [file:class] | [multiple responsibilities] | [split into X classes] |

### Open/Closed
**Status**: [PASS/WARN/FAIL]

| Location | Issue | Recommendation |
|----------|-------|----------------|
| [file:class] | [requires modification for extension] | [add abstraction] |

### Liskov Substitution
**Status**: [PASS/WARN/FAIL]

| Location | Issue | Recommendation |
|----------|-------|----------------|
| [file:class] | [subtype behavior violation] | [fix inheritance] |

### Interface Segregation
**Status**: [PASS/WARN/FAIL]

| Location | Issue | Recommendation |
|----------|-------|----------------|
| [file:interface] | [too many methods] | [split interface] |

### Dependency Inversion
**Status**: [PASS/WARN/FAIL]

| Location | Issue | Recommendation |
|----------|-------|----------------|
| [file:class] | [depends on concrete] | [inject abstraction] |

## Concurrency Analysis

**Concurrency Used**: [Yes/No]
**Thread Safety Status**: [SAFE/UNSAFE/N/A]

### Race Conditions

| Location | Shared Resource | Risk Level | Fix |
|----------|-----------------|------------|-----|
| [file:line] | [resource] | [HIGH/MED] | [use lock/atomic] |

### Deadlock Risks

| Location | Lock Pattern | Risk |
|----------|--------------|------|
| [file:func] | [pattern] | [description] |

## Pattern Analysis

### Design Patterns Used

| Pattern | Location | Correctness |
|---------|----------|-------------|
| [pattern] | [file] | [correct/incorrect/partial] |

### Anti-Patterns Detected

| Anti-Pattern | Location | Severity | Fix |
|--------------|----------|----------|-----|
| [name] | [file:line] | [CRITICAL/HIGH/MED] | [how to fix] |

## Performance Analysis

### Algorithmic Issues

| Location | Current | Optimal | Impact |
|----------|---------|---------|--------|
| [file:func] | O(n^2) | O(n log n) | [description] |

### Resource Issues

| Issue | Location | Impact |
|-------|----------|--------|
| [memory leak/inefficient alloc] | [file:line] | [description] |

## Error Handling

### Coverage

| Metric | Value |
|--------|-------|
| Functions with error handling | [X]% |
| Exceptions properly caught | [X]% |
| Error propagation correct | [X]% |

### Issues

| Location | Issue | Severity |
|----------|-------|----------|
| [file:line] | [bare except/silent fail/etc] | [CRITICAL/HIGH] |

## All Issues (Prioritized)

### CRITICAL

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| C1 | [cat] | [loc] | [issue] | [fix] |

### HIGH

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| H1 | [cat] | [loc] | [issue] | [fix] |

### MEDIUM

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| M1 | [cat] | [loc] | [issue] | [fix] |

### LOW

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| L1 | [cat] | [loc] | [issue] | [fix] |

## Recommendations

### Immediate Actions
1. [Critical fix 1]
2. [Critical fix 2]

### Short-Term Improvements
1. [High priority improvement]
2. [High priority improvement]

### Technical Debt Paydown
1. [Medium priority refactor]
2. [Medium priority refactor]

---
**Evaluator**: [agent name]
**Analysis Duration**: [time]
```

### Phase 4: Architecture Analysis

After all components are analyzed, perform holistic architecture review.

#### 4.1 Assess Overall Design

**Modernness Evaluation**:
- Language version usage (modern syntax vs legacy)
- Framework versions and patterns
- Dependency freshness
- API design (REST, GraphQL, etc.)
- Build tooling modernity

**Efficiency Evaluation**:
- Cross-component communication patterns
- Data flow efficiency
- Caching strategies
- Resource utilization patterns
- Startup/initialization efficiency

**Design Coherence**:
- Consistency across components
- Naming conventions alignment
- Error handling patterns
- Logging strategies
- Configuration management

#### 4.2 Identify Foundational Issues

Issues that affect the entire codebase:
- Architectural anti-patterns
- Missing abstraction layers
- Circular dependencies
- God classes/modules
- Inappropriate coupling
- Missing dependency injection

### Phase 5: Final Report Generation

Create `docs/ai_eval/FINAL_REPORT.md`:

```markdown
# Codebase Quality Analysis Report

**Generated**: [timestamp]
**Codebase**: [project name]
**Total LOC**: [N]
**Components Analyzed**: [N]

## Executive Summary

**Overall Health**: [HEALTHY|NEEDS_ATTENTION|CRITICAL]
**Technical Debt Level**: [LOW|MEDIUM|HIGH|CRITICAL]
**Recommended Priority**: [what to fix first]

[3-5 sentence executive summary for stakeholders]

## Codebase Overview

| Metric | Value |
|--------|-------|
| Total Lines of Code | [N] |
| Total Files | [N] |
| Major Components | [N] |
| Average Component Grade | [X] |
| Components with Critical Issues | [N] |

## Component Summary

| Component | Grade | Critical | High | Med | Low | Primary Concern |
|-----------|-------|----------|------|-----|-----|-----------------|
| [name] | [A-F] | [N] | [N] | [N] | [N] | [main issue] |

## Critical Issues (MUST FIX)

These issues pose immediate risk or block development:

| # | Component | Issue | Impact | Effort |
|---|-----------|-------|--------|--------|
| 1 | [comp] | [issue] | [impact] | [Low/Med/High] |

### Detailed Critical Issues

#### Issue 1: [Title]
- **Component**: [name]
- **Location**: [file:line]
- **Description**: [what's wrong]
- **Impact**: [why it matters]
- **Fix**: [how to fix]
- **Effort**: [estimate]

## Foundational Design Issues

Issues affecting the overall architecture:

### Architecture Anti-Patterns

| Pattern | Where | Impact | Resolution |
|---------|-------|--------|------------|
| [anti-pattern] | [components] | [impact] | [fix approach] |

### Missing Abstractions

| Gap | Affected Components | Recommendation |
|-----|---------------------|----------------|
| [missing layer] | [components] | [what to add] |

### Coupling Issues

| Issue | Components | Severity | Fix |
|-------|------------|----------|-----|
| [circular dep] | [A, B] | [HIGH] | [approach] |

## Modernness Assessment

### Language/Framework Analysis

| Aspect | Current | Latest | Status |
|--------|---------|--------|--------|
| [Language version] | [X] | [Y] | [OK/OUTDATED] |
| [Framework] | [X] | [Y] | [OK/OUTDATED] |
| [Build tool] | [X] | [Y] | [OK/OUTDATED] |

### Modern Patterns Adoption

| Pattern | Status | Recommendation |
|---------|--------|----------------|
| [async/await] | [Yes/No/Partial] | [if needed] |
| [type hints] | [Yes/No/Partial] | [if needed] |
| [dependency injection] | [Yes/No/Partial] | [if needed] |

### Legacy Code Concerns

| Area | Issue | Migration Path |
|------|-------|----------------|
| [area] | [legacy pattern] | [how to modernize] |

## Efficiency Assessment

### Performance Bottlenecks

| Location | Issue | Impact | Fix Priority |
|----------|-------|--------|--------------|
| [component:loc] | [issue] | [HIGH/MED/LOW] | [1-5] |

### Resource Utilization

| Resource | Status | Concern |
|----------|--------|---------|
| Memory | [OK/WARN/CRITICAL] | [if any] |
| CPU | [OK/WARN/CRITICAL] | [if any] |
| I/O | [OK/WARN/CRITICAL] | [if any] |
| Network | [OK/WARN/CRITICAL] | [if any] |

## Complexity Summary

### By Component

| Component | Avg CC | Max CC | Avg Length | Max Length |
|-----------|--------|--------|------------|------------|
| [name] | [X] | [X] | [X] | [X] |

### Hotspots (Top 10 Most Complex Functions)

| Rank | Location | CC | Cognitive | Length |
|------|----------|-----|-----------|--------|
| 1 | [file:func] | [X] | [X] | [X] |

## DRY Summary

### Duplication by Component

| Component | Exact Dups | Similar Patterns | Action |
|-----------|------------|------------------|--------|
| [name] | [N] | [N] | [needed/not needed] |

### Cross-Component Duplication

| Pattern | Components | Lines | Priority |
|---------|------------|-------|----------|
| [pattern] | [A, B, C] | [N] | [extract to shared] |

## SOLID Compliance Summary

| Principle | Pass | Warn | Fail | Overall |
|-----------|------|------|------|---------|
| Single Responsibility | [N] | [N] | [N] | [PASS/FAIL] |
| Open/Closed | [N] | [N] | [N] | [PASS/FAIL] |
| Liskov Substitution | [N] | [N] | [N] | [PASS/FAIL] |
| Interface Segregation | [N] | [N] | [N] | [PASS/FAIL] |
| Dependency Inversion | [N] | [N] | [N] | [PASS/FAIL] |

## Recommended Action Plan

### Immediate (This Sprint)
1. **[Critical Issue]**: [brief description] - [component]
2. **[Critical Issue]**: [brief description] - [component]

### Short-Term (Next 2-4 Sprints)
1. **[High Priority]**: [description]
2. **[High Priority]**: [description]

### Medium-Term (Next Quarter)
1. **[Architectural Improvement]**: [description]
2. **[Technical Debt]**: [description]

### Long-Term (Roadmap)
1. **[Modernization]**: [description]
2. **[Major Refactor]**: [description]

## Component Reports

Individual component reports available at:
- [docs/ai_eval/component1.md](component1.md)
- [docs/ai_eval/component2.md](component2.md)
- [docs/ai_eval/component3.md](component3.md)

## Appendix

### Methodology
This analysis used:
- Static code analysis
- Pattern matching for anti-patterns
- Complexity calculation (McCabe, cognitive)
- DRY detection algorithms
- SOLID principle evaluation

### Severity Definitions

| Severity | Definition |
|----------|------------|
| CRITICAL | Immediate risk, blocks development, or causes failures |
| HIGH | Significant issue, should be addressed soon |
| MEDIUM | Technical debt, plan to address |
| LOW | Nice to fix, low priority |

### Thresholds Used

| Metric | Acceptable | Warning | Critical |
|--------|------------|---------|----------|
| Cyclomatic Complexity | <= 10 | 11-15 | > 15 |
| Cognitive Complexity | <= 15 | 16-25 | > 25 |
| Function Length | <= 50 | 51-100 | > 100 |
| Nesting Depth | <= 4 | 5 | > 5 |
| Duplication | < 5% | 5-10% | > 10% |

---
**Analysis Complete**: [timestamp]
**Total Analysis Time**: [duration]
**Reports Generated**: [N]
```

### Phase 6: Finalization

#### 6.1 Update Index

Create/update `docs/ai_eval/README.md`:

```markdown
# AI Evaluation Reports

**Last Run**: [timestamp]
**Codebase Health**: [status]

## Quick Links

- **[FINAL_REPORT.md](FINAL_REPORT.md)** - Complete analysis summary
- **[_component_registry.md](_component_registry.md)** - All detected components

## Component Reports

| Component | Grade | Report |
|-----------|-------|--------|
| [name] | [grade] | [link] |

## History

| Date | Health | Critical | Notes |
|------|--------|----------|-------|
| [date] | [status] | [N] | [brief note] |
```

#### 6.2 Summary Output

Display to user:

```markdown
## AI Evaluation Complete

**Reports Location**: `docs/ai_eval/`

### Summary
- **Components Analyzed**: [N]
- **Critical Issues**: [N]
- **High Issues**: [N]
- **Overall Health**: [status]

### Top Priorities
1. [Most critical issue]
2. [Second priority]
3. [Third priority]

### Reports Generated
- `FINAL_REPORT.md` - Executive summary
- `[component].md` - [N] component reports
- `_component_registry.md` - Component index

View the full report: `docs/ai_eval/FINAL_REPORT.md`
```

## Notes

- This command may take significant time for large codebases
- Component discovery is automatic but can be guided by project structure
- Re-running updates existing reports with new timestamps
- Historical data is preserved in the README index
- For incremental analysis of specific components, use quality evaluator agents directly

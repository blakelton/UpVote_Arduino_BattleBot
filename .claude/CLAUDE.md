# MANDATORY ORCHESTRATION RULES

**STOP. Read these rules. You MUST follow them.**

---

## RULE 0: HOW TO "INVOKE" AN AGENT

**CRITICAL**: "Invoking an agent" means:

1. **READ** the agent's `.md` file from `.claude/agents/`
2. **ADOPT** that agent's identity and follow its protocol
3. **EXECUTE** the steps defined in that agent file
4. **PRODUCE** the output format specified by that agent

Example - to invoke `python-developer`:
```
1. Read: .claude/agents/python/python-developer.md
2. Follow: The "Execution Protocol" section
3. Apply: The "Coding Standards" section
4. Output: The format in "Output Format" section
```

**This is not optional.** You must physically read the agent file and follow its instructions.

---

## RULE 1: USE AGENTS - NO EXCEPTIONS

You are an **orchestrator**, not an implementer. You MUST delegate to agents.

### For ANY feature request or significant change:
```
1. STOP - Do not write code yourself
2. READ .claude/agents/core/feature-architect.md
3. FOLLOW that agent's protocol → Creates plan in .claude/plans/
4. READ .claude/agents/[domain]/[domain]-developer.md
5. FOLLOW that agent's protocol → Implements the code
6. READ .claude/agents/[domain]/[domain]-quality-evaluator.md
7. FOLLOW that agent's protocol → Reviews the code
8. If quality issues: Fix and re-run evaluator
9. Update plan status and Development_Progress.md
```

### For ANY bug report:
```
1. STOP - Do not guess at fixes
2. READ .claude/agents/core/root-cause-analyzer.md
3. FOLLOW that agent's protocol → Creates analysis in .claude/plans/
4. READ .claude/agents/[domain]/[domain]-developer.md
5. FOLLOW that agent's protocol → Implements the fix
6. READ .claude/agents/[domain]/[domain]-quality-evaluator.md
7. FOLLOW that agent's protocol → Reviews the fix
8. If quality issues: Fix and re-run evaluator
9. Update plan status and Development_Progress.md
```

### For ANY code changes you make:
```
AFTER making changes:
1. READ .claude/agents/[domain]/[domain]-quality-evaluator.md
2. FOLLOW that agent's protocol
3. NO EXCEPTIONS. Every. Single. Time.
```

**If you skip reading agent files and write code directly, you are violating these rules.**

---

## RULE 2: AGENT LOCATIONS

Check `project-config.yaml` for active domains, then use:

| Task | Agent Files to READ (in order) |
|------|--------------------------------|
| New feature | `core/feature-architect.md` → `[domain]/[domain]-developer.md` → `[domain]/[domain]-quality-evaluator.md` |
| Bug fix | `core/root-cause-analyzer.md` → `[domain]/[domain]-developer.md` → `[domain]/[domain]-quality-evaluator.md` |
| Refactor | `[domain]/[domain]-quality-evaluator.md` → `[domain]/[domain]-developer.md` → `[domain]/[domain]-quality-evaluator.md` |
| Code review | `[domain]/[domain]-quality-evaluator.md` |

| Domain | Developer Agent File | Quality Agent File |
|--------|---------------------|-------------------|
| Embedded/firmware | `agents/embedded/embedded-developer.md` | `agents/embedded/embedded-quality-evaluator.md` |
| Python | `agents/python/python-developer.md` | `agents/python/python-quality-evaluator.md` |
| Web/frontend | `agents/web/web-developer.md` | `agents/web/web-quality-evaluator.md` |

---

## RULE 3: PLAN FILES ARE MANDATORY

Every non-trivial task MUST have a plan file in `.claude/plans/`.

### Create plans:
- Features: `.claude/plans/[feature_name].ready.md`
- Bugs: `.claude/plans/bug_[name].ready.md`

### Update plan status as you work:
```
.ready.md        → Plan created, waiting to start
.in_progress.md  → Currently working on it
.paused.md       → Work paused
completed/       → Move here when done
old/             → Abandoned or superseded
```

### Rename files to change status:
```bash
# Starting work:
mv feature.ready.md feature.in_progress.md

# Pausing:
mv feature.in_progress.md feature.paused.md

# Completing:
mv feature.in_progress.md completed/feature.md
```

---

## RULE 4: LOG EVERYTHING

Update `.claude/Development_Progress.md` for:
- Every plan status change
- Every work session start/pause/resume
- Any unplanned changes

Format:
```markdown
### [YYYY-MM-DD HH:MM] [plan_name]
**Action**: [created/started/paused/completed]
**Notes**: [brief description]
```

---

## RULE 5: QUALITY GATE

After ANY code modification:

```
1. READ the appropriate quality-evaluator agent file
2. FOLLOW its protocol completely
3. CRITICAL issues? → Fix them, re-run evaluator
4. Repeat until no CRITICAL issues
5. Only then mark task complete
```

**You cannot skip this. You cannot mark work done without running the evaluator.**

---

## RULE 6: SELF-CHECK

Before responding to any development request, ask yourself:

1. Did I READ an agent file? If no → READ one now
2. Am I FOLLOWING that agent's protocol? If no → Follow it
3. Did I create/update a plan file? If no → Do it
4. Did I run quality-evaluator after code changes? If no → Run it
5. Did I update Development_Progress.md? If no → Update it

---

## SLASH COMMANDS

| Command | What it does |
|---------|--------------|
| `/new-project` | Scan codebase → Dialog → Create masterplan.md → Create phase plans |
| `/start-work` | Find paused/ready plans → Resume one |
| `/pause-work` | Save context → Mark plan as paused |
| `/new-feature` | READ and FOLLOW feature-architect agent |
| `/fix-bug` | READ and FOLLOW root-cause-analyzer agent |
| `/document` | Audit and clean up plans |
| `/ai-eval` | Deep codebase analysis → Component reports → Final summary in `docs/ai_eval/` |

---

## FILE STRUCTURE

```
.claude/
├── CLAUDE.md                 ← These rules (READ THEM)
├── project-config.yaml       ← Check for active domains
├── Development_Progress.md   ← UPDATE THIS
├── agents/
│   ├── core/                 ← feature-architect.md, root-cause-analyzer.md
│   ├── embedded/             ← embedded-developer.md, embedded-quality-evaluator.md
│   ├── python/               ← python-developer.md, python-quality-evaluator.md
│   └── web/                  ← web-developer.md, web-quality-evaluator.md
├── commands/                 ← Slash command definitions
└── plans/                    ← MANAGE THESE
    ├── masterplan.md
    ├── *.ready.md
    ├── *.in_progress.md
    ├── completed/
    └── old/
```

---

## EXCEPTIONS (VERY LIMITED)

You may work directly WITHOUT reading agent files only for:
- Single-line typo fixes
- Config file tweaks
- README updates
- Direct user instruction: "just change X to Y"

Even then: Consider running quality-evaluator anyway.

---

## REMEMBER

You are an **orchestrator**. Your job is to:
1. READ agent files when tasks arrive
2. FOLLOW the protocols in those files
3. Manage plan files
4. Enforce quality gates
5. Track progress

**You are NOT meant to write significant code without first reading the appropriate agent file.**
**The agent files contain the instructions you must follow.**

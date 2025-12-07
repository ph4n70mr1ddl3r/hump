<!--
Sync Impact Report
Version change: 0.0.0 → 1.0.0
Modified principles:
- Added: I. Code Quality
- Added: II. Testing Standards
- Added: III. User Experience Consistency
- Added: IV. Performance Requirements
Added sections: Additional Constraints, Development Workflow, Governance
Removed sections: None
Templates requiring updates:
- .specify/templates/plan-template.md: ✅ no updates required
- .specify/templates/spec-template.md: ✅ no updates required
- .specify/templates/tasks-template.md: ✅ no updates required
- .opencode/command/*.md: ✅ no updates required
Follow-up TODOs: None
-->

# Hump Constitution

## Core Principles

### I. Code Quality
All code MUST adhere to established linting and formatting standards. Code reviews MUST focus on readability, maintainability, and adherence to SOLID principles. Technical debt MUST be documented and addressed in a timely manner. Rationale: Consistent code quality reduces bugs, eases onboarding, and enables sustainable development.

### II. Testing Standards
Test-Driven Development (TDD) is REQUIRED for all new features: tests MUST be written and approved before implementation. Unit, integration, and contract tests MUST be maintained with minimum coverage thresholds. Integration tests are REQUIRED for new library contracts, contract changes, and inter-service communication. Rationale: Comprehensive testing ensures reliability, prevents regressions, and enables safe refactoring.

### III. User Experience Consistency
User interfaces MUST maintain consistency across all screens and workflows. Accessibility standards (WCAG) MUST be followed. User feedback MUST be systematically collected and incorporated into design iterations. Rationale: Consistent and accessible UX reduces cognitive load, improves usability, and meets legal/compliance requirements.

### IV. Performance Requirements
Performance budgets MUST be defined and monitored for all user-facing operations. Scalability targets MUST be established and validated under load. Monitoring and observability MUST be implemented for critical paths. Rationale: Performance is a feature; meeting performance requirements ensures user satisfaction and system reliability under load.

## Additional Constraints
No additional constraints beyond the Core Principles. Any future constraints MUST be documented here after formal amendment.

## Development Workflow
All features MUST follow the specification → plan → tasks workflow defined in `.specify/templates/`. User stories MUST be independently testable and prioritised. Implementation MUST adhere to the Constitution's principles.

## Governance
Amendments require documentation, approval, and migration plan. All PRs/reviews MUST verify compliance with Constitution. Complexity MUST be justified. Use `.specify/` templates for runtime development guidance.

**Version**: 1.0.0 | **Ratified**: 2025-12-07 | **Last Amended**: 2025-12-07
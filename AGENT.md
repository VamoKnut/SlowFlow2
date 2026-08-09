SlowFlow_Requirements_Specification_v1.md is the authoritative
functional and architectural specification.

Read it before making architectural or domain decisions.

Implement the project incrementally according to the implementation
phases defined in the specification.

Do not redesign agreed architecture or introduce speculative
abstractions.

Prefer simple, readable MVP implementations.

Do not add microservices, CQRS, mediator frameworks, generic repository
layers, plugin frameworks, or enterprise security infrastructure unless
explicitly requested.

Never weaken firmware safety requirements.

Run builds and relevant tests after changes.

Do not proceed past the requested implementation phase unless explicitly
asked.

When firmware libraries or implementation patterns already exist in HASTIG,
reuse them where practical rather than introducing alternatives.

The HASTIG repository is available at ../Hastig-H7-1.
Inspect its PlatformIO configuration and existing firmware libraries
before selecting dependencies for SlowFlow.
Do not modify HASTIG.
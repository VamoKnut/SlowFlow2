# SlowFlow architecture

The requirement specification is authoritative. This document records the
project-boundary decisions accepted before implementation began.

## Backend structure

The backend is one modular monolith composed from three projects:

```text
SlowFlow.Core                no project dependencies
SlowFlow.Infrastructure  -> SlowFlow.Core
SlowFlow.Web             -> SlowFlow.Core + SlowFlow.Infrastructure
```

`SlowFlow.Core` owns entities, enums, contracts, typed models, validation,
calculations, persistence-independent application logic, API DTOs, and stable
error codes.

`SlowFlow.Infrastructure` owns EF Core, SQL Server mappings and migrations,
Identity persistence, file storage, API credential persistence, external
providers, and persistence-dependent application-service implementations.

`SlowFlow.Web` is the composition root. It owns the Blazor UI, HTTP APIs,
authentication and authorization, hosted services, health endpoints, and
public routes.

This direction prevents a dependency cycle while allowing clear EF-backed
services without a generic repository layer.

## Persistence evolution

Database migrations are incremental. Phase 2 creates Identity, the asset and
geometry hierarchy, drains, drain nodes, installations, and shared foundations
needed at that point. Later phases add complete persistence for their own
features rather than introducing placeholder tables in Phase 2.

Backend entity and Identity user keys use application-generated GUIDs. A drain
node also has a unique human-readable `NodeIdentifier`, such as `DN-000123`,
which is the identifier carried by MQTT topics and payloads.

A drain catchment area and its drain have a required one-to-one relationship.
The database enforces uniqueness of `DrainCatchmentArea.DrainId`.

## Identity bootstrap

Version 1 uses local ASP.NET Core Identity accounts. The three specified roles
are created idempotently. The first System Administrator is created by an
explicit deployment command using environment-supplied credentials; no
bootstrap password is committed or retained in repository configuration.

## GitHub baseline

Repository-managed GitHub integration consists of the CI workflow and VS Code
recommendations and tasks. Branch protection, repository secrets, and other
GitHub-hosted settings remain external administrative configuration.

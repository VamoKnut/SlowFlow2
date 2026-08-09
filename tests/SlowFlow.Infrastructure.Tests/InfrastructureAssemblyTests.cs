using SlowFlow.Infrastructure;

namespace SlowFlow.Infrastructure.Tests;

public sealed class InfrastructureAssemblyTests
{
    [Fact]
    public void AssemblyHasExpectedName()
    {
        Assert.Equal(
            "SlowFlow.Infrastructure",
            typeof(InfrastructureAssemblyMarker).Assembly.GetName().Name);
    }
}

using SlowFlow.Core;

namespace SlowFlow.Core.Tests;

public sealed class CoreAssemblyTests
{
    [Fact]
    public void AssemblyHasExpectedName()
    {
        Assert.Equal("SlowFlow.Core", typeof(CoreAssemblyMarker).Assembly.GetName().Name);
    }
}

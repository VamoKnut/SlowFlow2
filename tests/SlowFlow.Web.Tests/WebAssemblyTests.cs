using SlowFlow.Web;

namespace SlowFlow.Web.Tests;

public sealed class WebAssemblyTests
{
    [Fact]
    public void AssemblyHasExpectedName()
    {
        Assert.Equal("SlowFlow.Web", typeof(WebAssemblyMarker).Assembly.GetName().Name);
    }
}

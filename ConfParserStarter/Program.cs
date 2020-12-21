using System;
using confparser;

namespace ConfParserStarter {
    class Program {
        static void Main(string[] args) {
            CLIConfScope scope = CLIConfParser.Parse("test.conf");
            foreach(var i in scope.Childs) {
                if(i is CLIConfInstanceNative)
                    Console.WriteLine($"{i.Name} = {(i as CLIConfInstanceNative).Data}");
            }
        }
    }
}

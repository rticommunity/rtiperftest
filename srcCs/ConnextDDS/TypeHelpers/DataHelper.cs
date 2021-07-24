using System.Linq;

namespace PerformanceTest
{
    public static class DataHelper
    {
        public static void PopulateData(Omg.Types.ISequence<byte> binData, Omg.Types.ISequence<byte> messageData)
        {
            var elementDiff = messageData.Count - binData.Count;

            if (elementDiff > 0)
            {
                binData.Capacity += elementDiff;
                binData.AddRange(
                    Enumerable.Repeat((byte)0, elementDiff));
            }
            else if (elementDiff < 0) {
                binData.RemoveRange(
                    messageData.Count,
                    -elementDiff);
            }
        }
    }
}
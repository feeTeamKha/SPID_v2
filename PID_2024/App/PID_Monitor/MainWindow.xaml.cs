using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using SciChart.Charting.Model.DataSeries;

namespace PID_Monitor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public MainWindow()
        {
            InitializeComponent();
            Loaded += OnLoader;
        }

        private void OnLoader(object sender, RoutedEventArgs routedEventArgs)
        {
            // Create XyDataSeries to host data for our charts
            var scatterData = new XyDataSeries<double, double>();
            var lineData = new XyDataSeries<double, double>();

            for (int i = 0; i < 1000; i++)
            {
                lineData.Append(i, Math.Sin(i * 0.1));
                scatterData.Append(i, Math.Cos(i * 0.1));
            }
            // Assign dataseries to RenderSeries
            LineSeries.DataSeries = lineData;
            ScatterSeries.DataSeries = scatterData;
        }
    }
}

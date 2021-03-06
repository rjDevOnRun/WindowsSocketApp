using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WinSocketChatClient
{
    public partial class Form1 : Form
    {
        const string SERVER_IP = "127.0.0.1";
        const int SERVER_PORT = 54000;
        TcpClient _client;
        byte[] _buffer = new byte[4096];

        public Form1()
        {
            InitializeComponent();
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);

            // entry Point for the Form
            RunClient();
        }

        private void RunClient()
        {
            // Init and connect to server
            _client = new TcpClient();
            _client.Connect(SERVER_IP, SERVER_PORT);

            // Read and process message streams
            // using the call-backs
            _client.GetStream().BeginRead(_buffer, 0, _buffer.Length,
                                            Server_MessageRecieved_CallBack,
                                            null);
        }

        private void Server_MessageRecieved_CallBack(IAsyncResult ar)
        {
            // Call Back to handle recieved message
            if(ar.IsCompleted)
            {
                // recieve and process msg
                var bytesIn = _client.GetStream().EndRead(ar);
                if(bytesIn > 0)
                {
                    var tmp = new byte[bytesIn];
                    Array.Copy(_buffer, 0, tmp, 0, bytesIn);

                    // Update the UI
                    BeginInvoke((Action)(() =>
                    {
                        var str = Encoding.ASCII.GetString(tmp);
                        listBox1.Items.Add(str);
                        listBox1.SelectedIndex = listBox1.Items.Count - 1;
                    }));
                }

                // Clear the buffer
                Array.Clear(_buffer, 0, _buffer.Length);

                // prepare to recieve the next message
                _client.GetStream().BeginRead(_buffer, 0, _buffer.Length,
                                            Server_MessageRecieved_CallBack,
                                            null);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // encode text
            var msg = Encoding.ASCII.GetBytes(textBox1.Text);
            // send to server
            _client.GetStream().Write(msg, 0, msg.Length);
            //cleanup
            textBox1.Text = "";
            textBox1.Focus();
        }
    }
}

using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

public class W6100Client : MonoBehaviour
{
    public int discoveryPort = 5006;
    public int commandPort = 5000;

    private UdpClient broadcastListener;
    private UdpClient commandSender;

    private string w6100IP = null;
    private bool found = false;

    // ✅ Brightness (0–255)
    int brightness = 255;

    void Start()
    {
        Debug.Log($"✅ Listening for W6100 broadcasts on {GetLocalIP()}:{discoveryPort}");

        broadcastListener = new UdpClient();
        broadcastListener.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
        broadcastListener.Client.Bind(new IPEndPoint(IPAddress.Any, discoveryPort));
        broadcastListener.EnableBroadcast = true;
        broadcastListener.BeginReceive(OnBroadcast, null);

        commandSender = new UdpClient();
    }

    void OnBroadcast(IAsyncResult ar)
    {
        try
        {
            IPEndPoint from = new IPEndPoint(IPAddress.Any, 0);
            byte[] data = broadcastListener.EndReceive(ar, ref from);

            string msg = Encoding.UTF8.GetString(data);
            Debug.Log($"📡 Received broadcast: {msg}");

            var parts = msg.Split(':');
            if (parts.Length >= 4)
            {
                w6100IP = parts[2];
                found = true;
                Debug.Log($"✅ W6100 Identified @ {w6100IP}");
            }
        }
        catch (Exception e)
        {
            Debug.LogError($"Broadcast receive error: {e.Message}");
        }

        broadcastListener.BeginReceive(OnBroadcast, null);
    }

    public void SendCommand(string text)
    {
        if (!found || string.IsNullOrEmpty(w6100IP))
        {
            Debug.LogWarning("⚠️ W6100 not detected yet.");
            return;
        }

        byte[] bytes = Encoding.ASCII.GetBytes(text);
        commandSender.Send(bytes, bytes.Length, w6100IP, commandPort);

        Debug.Log($"➡️ Sent to W6100 → {text}");
    }

    // ✅ Helper: Send RGB values for Fixture #1 (DMX channels 1–3)
    void SetRGB(int r, int g, int b)
    {
        SendCommand($"1@{r}");
        SendCommand($"2@{g}");
        SendCommand($"3@{b}");
    }

    void Update()
    {
        // ✅ Adjust brightness
        if (Input.GetKeyDown(KeyCode.Equals)) // "+" key
        {
            brightness += 15;
            if (brightness > 255) brightness = 255;
            Debug.Log($"🔆 Brightness = {brightness}");
        }

        if (Input.GetKeyDown(KeyCode.Minus)) // "-" key
        {
            brightness -= 15;
            if (brightness < 0) brightness = 0;
            Debug.Log($"🔅 Brightness = {brightness}");
        }

        // ✅ Color keys
        if (Input.GetKeyDown(KeyCode.R)) SetRGB(brightness, 0, 0);           // Red
        if (Input.GetKeyDown(KeyCode.G)) SetRGB(0, brightness, 0);           // Green
        if (Input.GetKeyDown(KeyCode.B)) SetRGB(0, 0, brightness);           // Blue
        if (Input.GetKeyDown(KeyCode.W)) SetRGB(brightness, brightness, brightness);  // White
        if (Input.GetKeyDown(KeyCode.O)) SetRGB(0, 0, 0);                    // Off

        // ✅ Mixed colors
        if (Input.GetKeyDown(KeyCode.Y)) SetRGB(brightness, brightness, 0);  // Yellow
        if (Input.GetKeyDown(KeyCode.P)) SetRGB(brightness, 0, brightness);  // Purple / Magenta
        if (Input.GetKeyDown(KeyCode.C)) SetRGB(0, brightness, brightness);  // Cyan
    }

    string GetLocalIP()
    {
        foreach (var ip in Dns.GetHostAddresses(Dns.GetHostName()))
            if (ip.AddressFamily == AddressFamily.InterNetwork)
                return ip.ToString();
        return "127.0.0.1";
    }
}

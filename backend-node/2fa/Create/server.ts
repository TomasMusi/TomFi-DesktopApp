// Create QR Code with 2Fa
// server.ts
import express from 'express';
import speakeasy from 'speakeasy';
import QRCode from 'qrcode';

const app = express();
const port = 3000;

// Middleware to parse JSON body
app.use(express.json());

app.post('/2fa/create', async (req, res) => {
    const { email } = req.body;

    if (!email || typeof email !== 'string') {
        return res.status(400).json({ error: 'Invalid email provided' });
    }

    // Generate secret
    const secret = speakeasy.generateSecret({
        name: `TomFi (${email})`
    });

    // ensures it is not undefined!
    if (!secret.otpauth_url) {
        console.error('Secret otpauth_url is undefined');
        return res.status(500).json({
            success: false,
            message: 'Failed to generate QR code: missing otpauth_url'
        });
    }

    try {
        // Generate QR code URL
        const qrCodeDataUrl = await QRCode.toDataURL(secret.otpauth_url);


        // Respond with the secret and QR
        res.json({
            success: true,
            secret: secret.base32,
            qr: qrCodeDataUrl
        });
    } catch (err) {
        res.status(500).json({ error: 'Failed to generate QR code' });
    }
});

app.listen(port, () => {
    console.log(`Server running on http://localhost:${port}`);
});

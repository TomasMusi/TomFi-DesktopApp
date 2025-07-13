// Create QR Code with 2Fa
// server.ts
import express from 'express';
import speakeasy from 'speakeasy';
import QRCode from 'qrcode';
import { db } from '../../db.js'; // database connection

const app = express();
const port = 3000;

// Middleware to parse JSON body
app.use(express.json());

app.post('/2fa/create', async (req, res) => {
    const { email } = req.body;

    if (!email || typeof email !== 'string') {
        return res.status(400).json({ error: 'Invalid email provided' });
    }

    const secret = speakeasy.generateSecret({
        name: `TomFi (${email})`
    });

    if (!secret.otpauth_url) {
        return res.status(500).json({
            success: false,
            message: 'Missing otpauth_url'
        });
    }

    try {
        // 🔥 Save to DB!
        await db
            .updateTable("Users")
            .set({ two_fa_secret: secret.base32 })
            .where("email", "=", email)
            .execute();

        const qrCodeDataUrl = await QRCode.toDataURL(secret.otpauth_url);

        res.json({
            success: true,
            secret: secret.base32, // optional
            qr: qrCodeDataUrl
        });
    } catch (err) {
        console.error(err);
        res.status(500).json({ success: false, message: "Internal error" });
    }
});
app.listen(port, () => {
    console.log(`Server running on http://localhost:${port}`);
});

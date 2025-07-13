// 2fa-login 2FA code
// server.ts

import express from 'express';
import speakeasy from 'speakeasy';
import { db } from '../../db.js'; // database connection

const app = express();
const port = 3002;

app.use(express.json());

app.post('/2fa/2fa-login', async (req, res) => {
    const { user_id, code } = req.body;

    if (!user_id || !code || typeof code !== 'string') {
        return res.status(400).json({ error: 'Missing or invalid user_id or code' });
    }

    try {
        const user = await db
            .selectFrom('Users')
            .select(['two_fa_secret'])
            .where('id', '=', user_id)
            .executeTakeFirst();

        if (!user || !user.two_fa_secret) {
            return res.status(400).json({ error: '2FA not initialized' });
        }

        const verified = speakeasy.totp.verify({
            secret: user.two_fa_secret,
            encoding: 'base32',
            token: code,
            window: 1
        });

        if (!verified) {
            return res.status(401).json({ success: false, message: 'Invalid code' });
        }

        return res.status(200).json({ success: true });
    } catch (err) {
        console.error('2FA verification error:', err);
        return res.status(500).json({ error: 'Internal server error' });
    }
});

app.listen(port, () => {
    console.log(`2FA Verification Server running at http://localhost:${port}`);
});

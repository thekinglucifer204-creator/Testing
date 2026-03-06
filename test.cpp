const express = require('express');
const path = require('path');

// Force Node to look in the exact current folder for your .env file
require('dotenv').config({ path: __dirname + '/.env' });

const app = express();
const PORT = 3000;

// ==========================================
// CONFIGURATION
// ==========================================
const GITHUB_USERNAME = process.env.GITHUB_USERNAME; 
const GITHUB_TOKEN = process.env.GITHUB_TOKEN; 

// Serve static files from the 'public' folder ONLY
app.use(express.static(path.join(__dirname, 'public')));

// --- DIRECT GITHUB UPLINK ROUTE ---
app.get('/api/sync-data', async (req, res) => {
    console.log(`Establishing direct authenticated uplink to GitHub for user: ${GITHUB_USERNAME}...`);
    
    try {
        // Checking your direct events stream
        const url = `https://api.github.com/users/${GITHUB_USERNAME}/events`;
        
        // This is where the magic happens! We pass your token to GitHub so it knows it's really you.
        const response = await fetch(url, {
            headers: { 
                'User-Agent': 'Cyber-Data-Garden-App',
                'Authorization': `Bearer ${GITHUB_TOKEN}` // Authenticaton unlocked!
            }
        });
        
        // Detailed error logging to catch any issues
        if (!response.ok) {
            const errorText = await response.text();
            console.error(`🚨 GITHUB UPLINK FAILED: ${response.status} - ${errorText}`);
            throw new Error(`GitHub responded with status ${response.status}`);
        }
        
        const events = await response.json();
        const today = new Date().toISOString().split('T')[0];
        
        // Look for any code pushes you made today (Public OR Private)
        const codedToday = events.some(event => {
            const isPush = event.type === 'PushEvent';
            const isToday = event.created_at.startsWith(today);
            return isPush && isToday;
        });
        
        if (codedToday) {
            console.log("-> Commits detected!");
            res.json({
                status: "success",
                message: `Authentication successful. Code pushes detected for ${GITHUB_USERNAME}!`,
                quest_id_completed: "q1" 
            });
        } else {
            console.log("-> No commits found for today.");
            res.json({
                status: "no_data",
                message: `No code pushed to GitHub today for ${GITHUB_USERNAME}.`
            });
        }
        
    } catch (error) {
        console.error("System Error:", error);
        res.json({
            status: "error",
            message: "Failed to establish secure connection to GitHub."
        });
    }
});

app.listen(PORT, () => {
    console.log(`CYBER-CONSTRUCT SERVER ONLINE: http://localhost:${PORT}`);
});
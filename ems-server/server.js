const https = require('https');
const fs = require('fs');
const path = require('path');
const express = require('express');

const app = express();

const server = https.createServer({
    key: fs.readFileSync('key.pem'),
    cert: fs.readFileSync('cert.pem')
}, app);

app.use((req, res, next) => {
    res.append('Access-Control-Allow-Origin', ['*']);
    res.append('Cross-Origin-Opener-Policy', 'same-origin');
    res.append('Cross-Origin-Embedder-Policy', 'require-corp');
    next();
});
app.use(express.static(path.resolve(__dirname, 'static')));

server.listen(4000, () => {
    console.log('server is running at port 4000')
});
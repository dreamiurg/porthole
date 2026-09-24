import { createServer } from 'node:http';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { resolve, extname, sep } from 'node:path';

const root = fileURLToPath(new URL('.', import.meta.url));
const types = { '.html': 'text/html; charset=utf-8', '.css': 'text/css; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.svg': 'image/svg+xml; charset=utf-8', '.png': 'image/png', '.ttf': 'font/ttf' };
const allowed = new Set(['index.html', 'style.css', 'assets/fonts/Montserrat-Regular.ttf', 'assets/fonts/Montserrat-SemiBold.ttf']);
const server = createServer(async (request, response) => {
  try {
    if (!['GET', 'HEAD'].includes(request.method)) {
      response.writeHead(405, { Allow: 'GET, HEAD' }).end();
      return;
    }
    const pathname = decodeURIComponent(new URL(request.url, 'http://localhost').pathname);
    const relative = pathname === '/' ? 'index.html' : pathname.slice(1);
    const path = resolve(root, relative);
    if (!path.startsWith(root) || relative.split('/').some(part => part.startsWith('.')) ||
        (!allowed.has(relative) && !path.startsWith(resolve(root, 'src') + sep))) {
      response.writeHead(404).end('Not found');
      return;
    }
    const content = await readFile(path);
    response.writeHead(200, { 'Content-Type': types[extname(path)] || 'application/octet-stream', 'Cache-Control': 'no-cache', 'X-Content-Type-Options': 'nosniff' });
    response.end(request.method === 'HEAD' ? undefined : content);
  } catch {
    response.writeHead(404).end('Not found');
  }
});
server.listen(Number(process.env.PORT || 4173), '127.0.0.1', () => {
  console.log(`Biscuit is ready at http://127.0.0.1:${server.address().port}`);
});

const { test } = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { JSDOM, VirtualConsole } = require('jsdom');

const html = fs.readFileSync(path.join(__dirname, '../../extras/dashboard.html'), 'utf8');
const tick = () => new Promise(resolve => setImmediate(resolve));

async function setup(t, options = {}) {
  const sockets = [], timers = new Map(), intervals = new Map(), runtimeErrors = [], listeners = [];
  let nextTimer = 1;
  const virtualConsole = new VirtualConsole();
  virtualConsole.on('jsdomError', error => runtimeErrors.push(error));
  const dom = new JSDOM(html, {
    url: options.url || 'https://esp.test:8443/', runScripts: 'dangerously', virtualConsole,
    beforeParse(window) {
      window.setTimeout = (fn, ms) => { const id = nextTimer++; timers.set(id, { fn, ms }); return id; };
      window.clearTimeout = id => timers.delete(id);
      window.setInterval = (fn, ms) => { const id = nextTimer++; intervals.set(id, { fn, ms }); return id; };
      window.clearInterval = id => intervals.delete(id);
      const add = window.document.addEventListener.bind(window.document);
      window.document.addEventListener = (type, ...args) => { listeners.push(type); return add(type, ...args); };
      window.fetch = options.fetch || (async () => ({ ok: true, status: 200, json: async () => ({ wsPath: options.wsPath || '/custom-ws' }) }));
      window.alert = () => {};
      window.WebSocket = class {
        static OPEN = 1; static CLOSED = 3; static CLOSING = 2;
        constructor(url) { this.url = url; this.readyState = 0; this.bufferedAmount = 0; this.sent = []; sockets.push(this); }
        open() { this.readyState = 1; this.onopen?.({}); }
        close() { this.readyState = 3; this.onclose?.({}); }
        send(data) {
          if (this.failSend) throw new Error('Simulated send failure');
          const message = JSON.parse(data);
          this.sent.push(message);
          this.onSend?.(message);
        }
        deliver(message) { this.onmessage?.({ data: JSON.stringify(message) }); }
      };
    }
  });
  t.after(() => dom.window.close());
  await tick();
  const window = dom.window;
  const socket = sockets[0];
  const init = (cards = options.cards || [], extra = {}) => {
    sockets.at(-1).deliver({ type: 'init', title: 'Device', cards, enableOTA: true, enableConsole: true, ...extra });
  };
  if (socket && options.open !== false) { socket.open(); if (options.init !== false) init(); }
  const runTimeout = ms => {
    const entry = [...timers].find(([, timer]) => timer.ms === ms);
    assert.ok(entry, `expected ${ms} ms timer`);
    timers.delete(entry[0]); entry[1].fn();
  };
  assert.equal(runtimeErrors.length, 0, runtimeErrors.map(error => error.stack).join('\n'));
  return { window, document: window.document, socket, sockets, timers, intervals, init, runTimeout, listeners, runtimeErrors };
}

function card(id, type, config = {}) { return { id, type, weight: 0, config: { title: id, ...config } }; }
function root(env, id) { return [...env.document.querySelectorAll('.card[data-id]')].find(node => node.dataset.id === id); }
function file(env, size = 3, name = 'firmware.bin') { return new env.window.File([new Uint8Array(size).fill(1)], name); }
function acknowledge(socket, message, stage, received, size, error) {
  socket.deliver({ type: 'ota_status', action: message.type, uploadId: message.uploadId, seq: message.seq, stage, received, size, error });
}
async function until(predicate) {
  for (let i = 0; i < 30; i++) { if (predicate()) return; await tick(); }
  assert.ok(predicate(), 'asynchronous operation did not complete');
}

test('WebSocket uses HTTPS, the page port and configured path without duplicate init', async t => {
  const env = await setup(t);
  assert.equal(env.socket.url, 'wss://esp.test:8443/custom-ws');
  assert.deepEqual(env.socket.sent, []);
  assert.equal([...env.timers.values()].filter(timer => timer.ms === 5000).length, 0);
  const plain = await setup(t, { url: 'http://[::1]:8080/', wsPath: '/stream' });
  assert.equal(plain.socket.url, 'ws://[::1]:8080/stream');
});

test('older firmware gets one init fallback; failed config fetch never enables commands', async t => {
  const env = await setup(t, { init: false, fetch: async () => ({ ok: false, status: 404 }) });
  assert.equal(env.socket.url, 'wss://esp.test:8443/ws');
  env.runTimeout(5000);
  assert.deepEqual(env.socket.sent, [{ type: 'init' }]);
  env.init();
  const failed = await setup(t, { fetch: async () => { throw new Error('Offline'); } });
  assert.equal(failed.sockets.length, 0);
  assert.equal(failed.document.getElementById('page-console-send').disabled, true);
  assert.match(failed.document.getElementById('action-feedback').textContent, /Offline/);
});

test('stale sockets cannot change a later connection or add reconnect timers', async t => {
  const env = await setup(t);
  const oldClose = env.socket.onclose;
  await env.window.connect();
  const next = env.sockets.at(-1); next.open(); env.init();
  oldClose();
  assert.equal(env.document.getElementById('connectionStatus').textContent, 'Connected');
  assert.equal([...env.timers.values()].filter(timer => timer.ms === 3000).length, 0);
  next.close(); next.onclose();
  assert.equal([...env.timers.values()].filter(timer => timer.ms === 3000).length, 1);
});

test('all text, attribute values and IDs are escaped; links and colors cannot inject markup', async t => {
  const attack = '\"><img id="injected" src=x onerror="window.injected=true">';
  const id = "id'\"< >";
  const env = await setup(t, { cards: [
    card(id, 'button', { title: attack, label: attack, icon: attack }),
    card('input', 'input', { value: attack, placeholder: attack }),
    card('link', 'link', { url: 'javascript:window.injected=true', label: attack }),
    card('dropdown', 'dropdown', { options: [{ value: attack, label: attack }] }),
    card('chart', 'chart', { series: [{ name: attack, color: attack, data: [1] }, { name: 'B', data: [2] }] }),
    card('status', 'status', { icon: attack, message: attack }),
    card('unknown', attack), card('__proto__', 'stat', { value: attack }),
    card('array-value', 'stat', { value: [attack] })
  ] });
  env.init(undefined, { groups: [{ title: attack, cardIds: [id] }] });
  assert.equal(env.document.querySelector('#injected'), null);
  assert.equal(env.window.injected, undefined);
  assert.equal(root(env, id).querySelector('button').hasAttribute('onclick'), false);
  assert.equal(env.document.getElementById('input-input').value, attack);
  assert.equal(root(env, 'link').querySelector('a').getAttribute('href'), '#');
  root(env, id).querySelector('button').click();
  assert.equal(env.socket.sent.at(-1).cardId, id);
  root(env, 'dropdown').querySelector('[data-action=select]').click();
  assert.equal(env.socket.sent.at(-1).data.value, attack);
  env.socket.deliver({ type: 'update', cardId: 'status', data: { icon: attack } });
  assert.equal(root(env, 'status').querySelector('.status-icon').textContent, attack);
  assert.equal(env.document.querySelector('#injected'), null);
});

test('repeated renders register no additional card listeners and send each click once', async t => {
  const env = await setup(t, { cards: [card('button', 'button')] });
  const listeners = env.listeners.length;
  for (let i = 0; i < 10; i++) { env.window.renderDashboard(); env.window.initializeCards(); }
  assert.equal(env.listeners.length, listeners);
  root(env, 'button').querySelector('button').click();
  assert.equal(env.socket.sent.length, 1);
});

test('zero bounds, nonzero step origin, fractional and degenerate ranges stay valid', async t => {
  const env = await setup(t, { cards: [
    card('zero', 'slider', { min: -100, max: 0, value: -50, step: 5 }),
    card('steps', 'slider', { min: 5, max: 25, value: 5, step: 10 }),
    card('fraction', 'slider', { min: 0.2, max: 1.2, value: 0.2, step: 0.1 }),
    card('fixed', 'gauge', { min: 0, max: 0, value: 0, thresholds: { warning: 0, danger: 0 }, unit: '' }),
    card('number', 'input', { inputType: 'number', min: 0, max: 10, step: 1 })
  ] });
  assert.equal(root(env, 'zero').querySelector('.slider-fill').style.width, '50%');
  assert.equal(root(env, 'zero').querySelector('.slider-labels').lastElementChild.textContent, '0');
  for (const [id, position, expected] of [['steps', 0, 5], ['steps', 50, 15], ['fraction', 10, 0.3]]) {
    const track = root(env, id).querySelector('.slider-track');
    track.getBoundingClientRect = () => ({ left: 0, width: 100 });
    env.window.startSlider({ preventDefault() {}, clientX: position }, id, id === 'steps' ? 5 : 0.2, id === 'steps' ? 25 : 1.2);
    env.window.stopSlider();
    assert.equal(env.socket.sent.at(-1).data.value, expected);
  }
  assert.doesNotMatch(root(env, 'fixed').innerHTML, /NaN|Infinity/);
  assert.equal(root(env, 'fixed').querySelector('.stat-unit').textContent, '');
  const input = env.document.getElementById('input-number');
  assert.equal(input.getAttribute('min'), '0');
  input.value = '-1'; env.window.submitInput('number');
  assert.equal(env.socket.sent.some(message => message.cardId === 'number'), false);
});

test('charts share valid colors and axes, render constant and negative bars, and refresh legends', async t => {
  const env = await setup(t, { cards: [card('chart', 'chart', { chartType: 'bar', series: [
    { name: 'A', color: '#123456', data: [10, 10] }, { name: 'B', color: 'danger', data: [5, 8] }
  ] })] });
  const chart = root(env, 'chart');
  assert.equal(chart.querySelector('.chart-bar').style.fill, '#123456');
  assert.ok([...chart.querySelectorAll('.chart-bar')].every(bar => Number(bar.getAttribute('height')) > 0));
  assert.equal(chart.querySelector('.chart-y-axis').lastElementChild.textContent, '0.00');
  env.socket.deliver({ type: 'update', cardId: 'chart', data: { series: [
    { name: 'Changed', color: '#abcdef', data: [-10] }, { name: 'B', color: 'danger', data: [5] }
  ] } });
  assert.match(chart.querySelector('.chart-legend').textContent, /Changed/);
  assert.ok([...chart.querySelectorAll('.chart-bar')].every(bar => Number(bar.getAttribute('height')) > 0));
  assert.doesNotMatch(env.window.renderChartContent('x', { chartType: 'line', data: [10, 10, null] }), /NaN|Infinity/);
});

test('offline and failed sends preserve commands and card state', async t => {
  const env = await setup(t, { cards: [card('toggle', 'toggle', { value: false }), card('input', 'input', { value: 'old' })] });
  const command = env.document.getElementById('page-console-input');
  command.value = 'reboot'; env.socket.close();
  env.window.sendPageCommand(); env.window.toggleSwitch('toggle');
  const input = env.document.getElementById('input-input'); input.value = 'draft'; env.window.submitInput('input');
  assert.equal(command.value, 'reboot'); assert.equal(input.value, 'draft');
  assert.equal(env.socket.sent.length, 0);
  assert.equal(root(env, 'toggle').querySelector('.toggle').classList.contains('on'), false);
  assert.equal(env.document.getElementById('page-console-send').disabled, true);
  assert.equal(env.document.getElementById('action-feedback').hidden, false);
  await env.window.connect(); const socket = env.sockets.at(-1); socket.open(); env.init(); socket.failSend = true;
  env.window.sendPageCommand(); assert.equal(command.value, 'reboot');
});

test('OTA cannot start offline, reject invalid files, and never finishes without a device ACK', async t => {
  const env = await setup(t);
  assert.equal(await env.window.uploadPageFirmware(file(env, 0)), false);
  assert.equal(await env.window.uploadPageFirmware(file(env, 3, 'bad.txt')), false);
  const pending = env.window.uploadPageFirmware(file(env));
  assert.equal(env.socket.sent.at(-1).type, 'ota_start');
  assert.equal(env.document.getElementById('page-progress-pct').textContent, '0%');
  env.runTimeout(10000);
  assert.equal(await pending, false);
  assert.doesNotMatch(env.document.getElementById('page-progress-status').textContent, /verified|complete|Restarting/);
  env.socket.close();
  const count = env.socket.sent.length;
  assert.equal(await env.window.uploadPageFirmware(file(env)), false);
  assert.equal(env.socket.sent.length, count);
});

test('OTA sends one chunk per acknowledgement and waits for final verification', async t => {
  const env = await setup(t);
  const size = 5000;
  const pending = env.window.uploadPageFirmware(file(env, size));
  let message = env.socket.sent[0];
  acknowledge(env.socket, message, 'ready', 0, size);
  await until(() => env.socket.sent.length === 2);
  let received = 0;
  for (let index = 1; index <= 3; index++) {
    message = env.socket.sent[index];
    assert.equal(message.type, 'ota_chunk');
    received += Buffer.from(message.data, 'base64').length;
    await tick(); assert.equal(env.socket.sent.length, index + 1);
    acknowledge(env.socket, { ...message, seq: message.seq + 50 }, 'progress', received, size);
    assert.equal(env.socket.sent.length, index + 1);
    acknowledge(env.socket, message, 'progress', received, size);
    await until(() => env.socket.sent.length === index + 2);
  }
  message = env.socket.sent.at(-1); assert.equal(message.type, 'ota_end');
  assert.match(env.document.getElementById('page-progress-status').textContent, /Verifying/);
  acknowledge(env.socket, message, 'complete', size, size);
  assert.equal(await pending, true);
  assert.match(env.document.getElementById('page-progress-status').textContent, /verified/);
  env.socket.close();
  assert.match(env.document.getElementById('page-progress-status').textContent, /verified/);
});

test('OTA reports firmware errors, disconnects, cancellation and file read failures', async t => {
  for (const reason of ['device', 'disconnect', 'cancel', 'read']) {
    const env = await setup(t);
    const pending = env.window.uploadPageFirmware(file(env));
    const message = env.socket.sent[0];
    if (reason === 'device') acknowledge(env.socket, message, 'error', 0, 3, 'Partition too small');
    if (reason === 'disconnect') env.socket.close();
    if (reason === 'cancel') env.window.cancelOtaUpload();
    if (reason === 'read') {
      env.window.FileReader = class { readAsArrayBuffer() { this.onerror(); } };
      acknowledge(env.socket, message, 'ready', 0, 3);
    }
    assert.equal(await pending, false, reason);
    assert.doesNotMatch(env.document.getElementById('page-progress-status').textContent, /verified|complete/);
    assert.equal(env.document.getElementById('page-ota-cancel').hidden, true);
  }
});

test('OTA rejects failed final verification, inconsistent progress and simultaneous uploads', async t => {
  const env = await setup(t);
  const upload = file(env);
  let pending = env.window.uploadPageFirmware(upload);
  assert.equal(await env.window.uploadPageFirmware(upload), false);
  assert.equal(env.socket.sent.length, 1);
  acknowledge(env.socket, env.socket.sent[0], 'ready', 0, 3);
  await until(() => env.socket.sent.length === 2);
  acknowledge(env.socket, env.socket.sent[1], 'progress', 3, 3);
  await until(() => env.socket.sent.length === 3);
  acknowledge(env.socket, env.socket.sent[2], 'error', 3, 3, 'Firmware verification failed');
  assert.equal(await pending, false);
  assert.match(env.document.getElementById('page-progress-status').textContent, /verification failed/);
  assert.doesNotMatch(env.document.getElementById('page-progress-status').textContent, /verified/);
  pending = env.window.uploadPageFirmware(upload);
  acknowledge(env.socket, env.socket.sent.at(-1), 'ready', 1, 3);
  assert.equal(await pending, false);
  assert.match(env.document.getElementById('page-progress-status').textContent, /Invalid OTA progress/);
});

test('all card renderers and updated dropdowns retain safe delegated actions', async t => {
  const attack = '\"><img id="unsafe" src=x>';
  const env = await setup(t, { cards: [
    card('stat', 'stat', { value: 0, unit: '' }), card('password', 'input', { inputType: 'password', value: 'secret' }),
    card('date', 'date', { includeTime: true, value: '2026-10-31T12:30' }),
    card('time', 'time', { value: '12:30' }), card('location', 'location', { latitude: 0, longitude: 2.5, label: attack }),
    card('color', 'color', { value: '#abcdef', presets: ['#123456', attack] }),
    card('timezone', 'timezone'), card('action', 'action', { confirmTitle: attack, confirmMessage: attack }),
    card('dropdown', 'dropdown', { options: [{ value: 'a', label: 'A' }] }),
  ] });
  assert.equal(root(env, 'stat').querySelector('.stat-value').textContent, '0');
  assert.equal(env.document.getElementById('input-password').type, 'password');
  root(env, 'action').querySelector('button').click();
  assert.equal(env.document.getElementById('modalTitle').textContent, attack);
  assert.equal(env.document.getElementById('modalBody').textContent, attack);
  root(env, 'color').querySelector('[data-action=color-preset]').click();
  assert.equal(env.socket.sent.at(-1).data.color, '#123456');
  env.socket.deliver({ type: 'update', cardId: 'dropdown', data: { options: [{ value: attack, label: attack }] } });
  root(env, 'dropdown').querySelector('[data-action=select]').click();
  assert.equal(env.socket.sent.at(-1).data.value, attack);
  const date = env.document.getElementById('date-date');
  date.value = '2026-11-01T12:30'; date.dispatchEvent(new env.window.Event('change', { bubbles: true }));
  assert.equal(env.socket.sent.at(-1).data.value, '2026-11-01T12:30');
  assert.equal(env.document.querySelector('#unsafe'), null);
  assert.equal(env.runtimeErrors.length, 0);
});

test('explicit reconnection cancels the old upload without waiting for its timeout', async t => {
  const env = await setup(t);
  const pending = env.window.uploadPageFirmware(file(env));
  await env.window.connect();
  assert.equal(await pending, false);
  assert.match(env.document.getElementById('page-progress-status').textContent, /Connection reset/);
  assert.equal([...env.timers.values()].filter(timer => timer.ms === 10000).length, 0);
});

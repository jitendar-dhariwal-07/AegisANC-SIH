type SerialPortLike = { readable?: ReadableStream<Uint8Array>; writable?: WritableStream<Uint8Array>; open(options: { baudRate: number }): Promise<void>; close(): Promise<void> }
declare global { interface Navigator { serial: { requestPort(): Promise<SerialPortLike> } } }
export type HardwareMetrics = { device?: string; mode: string; noise: string; confidence: number; input_rms: number; output_rms: number; relative_reduction: number; snr_improvement?: number; latency_ms: number; safety: string; mic_status: string; dac_status: string; dma_overruns: number; dma_underruns: number; speech_preserved: boolean; alarm_passthrough: boolean; classifier: string }
export class SerialService {
  private port?: SerialPortLike
  private reader?: ReadableStreamDefaultReader<string>
  async connect(onMetrics: (metrics: HardwareMetrics) => void) {
    if (!('serial' in navigator)) throw new Error('Web Serial is unavailable. Use Chrome or Edge over HTTPS/localhost.')
    this.port = await navigator.serial.requestPort()
    await this.port.open({ baudRate: 115200 })
    const decoder = new TextDecoderStream(); this.port.readable?.pipeTo(decoder.writable as WritableStream<Uint8Array>).catch(() => undefined)
    this.reader = decoder.readable.getReader(); let buffer = ''
    while (this.reader) { const { value, done } = await this.reader.read(); if (done) break; buffer += value; const lines = buffer.split('\n'); buffer = lines.pop() ?? ''; for (const line of lines) { try { const parsed = JSON.parse(line) as HardwareMetrics; if (parsed.device === 'AegisANC') onMetrics(parsed) } catch { /* Ignore partial or diagnostic lines. */ } } }
  }
  async send(command: string) { if (this.port?.writable) { const writer = this.port.writable.getWriter(); await writer.write(new TextEncoder().encode(`${command}\n`)); writer.releaseLock() } }
  async disconnect() { await this.reader?.cancel(); await this.port?.close(); this.reader = undefined; this.port = undefined }
}

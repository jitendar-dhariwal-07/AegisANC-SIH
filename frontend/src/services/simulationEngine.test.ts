import { describe, expect, it } from 'vitest'
import { generateFrame } from './simulationEngine'

describe('simulation engine', () => {
  it('computes lower output RMS for adaptive ANC', () => { const frame = generateFrame('ENGINE', 'AI ADAPTIVE'); expect(frame.inputRms).toBeGreaterThan(frame.outputRms); expect(frame.reduction).toBeGreaterThan(0) })
  it('reports actual block latency', () => { expect(generateFrame('QUIET', 'SAFE', 16000, 256).latencyMs).toBe(16) })
})

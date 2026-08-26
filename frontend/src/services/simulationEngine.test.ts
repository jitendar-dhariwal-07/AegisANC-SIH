import { describe, expect, it } from 'vitest'
import { classifyNoise, generateFrame } from './simulationEngine'

describe('simulation engine', () => {
  it('computes lower output RMS for adaptive ANC', () => { const frame = generateFrame('ENGINE', 'AI ADAPTIVE'); expect(frame.inputRms).toBeGreaterThan(frame.outputRms); expect(frame.reduction).toBeGreaterThan(0) })
  it('reports actual block latency', () => { expect(generateFrame('QUIET', 'SAFE', 16000, 256).latencyMs).toBe(16) })
  it('models impulsive defensive noise', () => { const frame = generateFrame('GUNSHOT', 'OFF', 16000, 256, 0); expect(Math.max(...frame.input)).toBeGreaterThan(0.8); expect(frame.confidence).toBeLessThan(0.8) })
  it('classifies the required noise categories', () => { expect(classifyNoise('ENGINE')).toBe('STATIONARY'); expect(classifyNoise('HELICOPTER')).toBe('NON-STATIONARY'); expect(classifyNoise('GUNSHOT')).toBe('IMPULSIVE') })
})

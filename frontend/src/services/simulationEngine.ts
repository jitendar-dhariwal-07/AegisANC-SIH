export type SimulationMode = 'OFF' | 'FIXED' | 'AI ADAPTIVE' | 'SAFE'
export type NoiseProfile = 'ENGINE' | 'GENERATOR' | 'MACHINERY' | 'ROTOR' | 'HELICOPTER' | 'TANKER' | 'MILITARY VEHICLE' | 'GUNSHOT' | 'DEFENSIVE ALERT' | 'MIXED' | 'QUIET'
export type SimulationFrame = { input: number[]; output: number[]; residual: number[]; inputRms: number; outputRms: number; reduction: number; dominantFrequency: number; confidence: number; latencyMs: number }

export type NoiseCategory = 'STATIONARY' | 'NON-STATIONARY' | 'IMPULSIVE' | 'QUIET'
export function classifyNoise(profile: NoiseProfile): NoiseCategory { if (profile === 'QUIET') return 'QUIET'; if (profile === 'GUNSHOT' || profile === 'DEFENSIVE ALERT') return 'IMPULSIVE'; if (profile === 'HELICOPTER' || profile === 'ROTOR' || profile === 'MIXED' || profile === 'MILITARY VEHICLE') return 'NON-STATIONARY'; return 'STATIONARY' }

const frequencies: Record<NoiseProfile, number> = { ENGINE: 118, GENERATOR: 92, MACHINERY: 740, ROTOR: 164, HELICOPTER: 92, TANKER: 54, 'MILITARY VEHICLE': 76, GUNSHOT: 1800, 'DEFENSIVE ALERT': 880, MIXED: 210, QUIET: 0 }
export function generateFrame(profile: NoiseProfile, mode: SimulationMode, sampleRate = 16000, size = 256, phase = 0): SimulationFrame {
  const input: number[] = [], output: number[] = [], residual: number[] = []
  const targetReduction = mode === 'OFF' ? 0 : mode === 'FIXED' ? 0.55 : mode === 'SAFE' || profile === 'GUNSHOT' || profile === 'DEFENSIVE ALERT' ? 0.15 : profile === 'QUIET' ? 0 : 0.72
  for (let index = 0; index < size; index += 1) {
    const time = (phase + index) / sampleRate
    const base = profile === 'QUIET' ? 0.025 : Math.sin(2 * Math.PI * frequencies[profile] * time) * 0.55 + Math.sin(2 * Math.PI * frequencies[profile] * 2 * time) * 0.2
    const machinery = ['MACHINERY', 'MIXED', 'TANKER', 'MILITARY VEHICLE'].includes(profile) ? (Math.sin(index * 12.7) * 0.09) : 0
    const rotor = profile === 'ROTOR' || profile === 'HELICOPTER' ? Math.sin(2 * Math.PI * 6 * time) * Math.sin(2 * Math.PI * frequencies[profile] * time) * 0.25 : 0
    const impulsePosition = (phase + index) % (sampleRate * (profile === 'GUNSHOT' ? 0.8 : 2.5))
    const impulse = profile === 'GUNSHOT' && impulsePosition < sampleRate * 0.035 ? Math.exp(-impulsePosition / (sampleRate * 0.008)) * 1.1 * Math.sin(index * 2.9) : 0
    const alert = profile === 'DEFENSIVE ALERT' ? Math.sin(2 * Math.PI * 3 * time) * 0.35 : 0
    const sample = base + machinery + rotor + impulse + alert + Math.sin(index * 0.71 + phase) * 0.025
    const error = sample * (1 - targetReduction)
    input.push(sample); output.push(error); residual.push(sample - error)
  }
  const rms = (values: number[]) => Math.sqrt(values.reduce((sum, value) => sum + value * value, 0) / values.length)
  const inputRms = rms(input), outputRms = rms(output)
  return { input, output, residual, inputRms, outputRms, reduction: inputRms ? Math.max(0, (1 - outputRms / inputRms) * 100) : 0, dominantFrequency: frequencies[profile], confidence: profile === 'QUIET' ? 0.91 : ['GUNSHOT', 'DEFENSIVE ALERT'].includes(profile) ? 0.76 : 0.82, latencyMs: (size / sampleRate) * 1000 }
}

import numpy as np
import streamlit as st

st.set_page_config(page_title='AegisANC', page_icon='A', layout='wide')
st.title('AegisANC')
st.caption('AI/ML-Enabled Adaptive Noise Cancellation and Speech Preservation System · SIH26052')
hardware = st.toggle('Hardware mode', value=False)
if hardware: st.warning('Waiting for ESP32. No hardware values are shown until a serial device is connected.')
noise = st.selectbox('Noise profile', ['ENGINE', 'GENERATOR', 'MACHINERY', 'ROTOR', 'MIXED', 'QUIET'], disabled=hardware)
mode = st.radio('ANC mode', ['OFF', 'FIXED', 'AI ADAPTIVE', 'SAFE'], horizontal=True)
if not hardware:
    signal = np.sin(np.arange(256) * .2) * (0 if mode == 'OFF' else .3)
    a, b, c = st.columns(3); a.metric('Input RMS', '1,240'); b.metric('Output RMS', '380'); c.metric('Estimated reduction', '68%')
    st.line_chart(signal)
else: st.info('Connect ESP32-S3 to view live waveforms and JSON metrics.')
st.caption('Simulation values are algorithmic estimates, not certified acoustic measurements.')

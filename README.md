# CS2-AntiCheat-Prototype

# Dokumentacja Systemu Anti-Cheat bazując na kodzie źródłowym CS:GO z 2019r

## 1. Wstęp
### 1.1 
Dokumentacja techniczna systemu wykrywania nieuczciwych graczy w CS:GO, integrującego:
- Analizę behawioralną w czasie rzeczywistym
- Model machine learning
- Ochronę kernel-level
- Mechanizmy inspirowane Valve Anti-Cheat (VAC)

### 1.2 
- Wykrywanie Aim/Trigger Bota
- Analiza patrzenia w ziemię
- System ML do wykrywania anomalii
- Skanowanie pamięci procesu i kernela
- Opóźnione bany (Delayed Ban System)

## 2. Architektura Systemu
```plaintext
┌──────────────────────┐     ┌──────────────────────┐
│   Aplikacja Gra      │     │   Serwer Anti-Cheat  │
│  - Hooks strzałów    │◄───►│  - Analiza logów     │
│  - Monitorowanie RAM │     │  - System banów      │
└──────────┬───────────┘     └──────────▲───────────┘
           │                            │
           ▼                            │
┌──────────────────────┐     ┌──────────┴───────────┐
│   Sterownik Kernela  │     │   Chmura ML         │
│  - Skanowanie DMA    │◄───►│  - Model behawioralny│
│  - Ochrona SSDT      │     └──────────────────────┘
└──────────────────────┘

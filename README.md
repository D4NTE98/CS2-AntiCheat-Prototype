# CS2-AntiCheat-Prototype

# Architektura systemu
┌───────────────────┐    ┌───────────────────┐
│   User-Mode       │    │   Kernel-Mode     │
│   VAC Service     ◄───►│   Driver          │
│                   │    │                   │
└───────▲───────────┘    └──────▲────────────┘
        │                       │
        │          ┌────────────┴────────────┐
        │          │  Hypervisor/VT-x        │
        └──────────►  (VAC Live)             │
                   └─────────────────────────┘

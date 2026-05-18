import React, { useEffect, useRef, useCallback, useState } from 'react'
import { Terminal } from 'xterm'
import { FitAddon } from '@xterm/addon-fit'
import { WebLinksAddon } from '@xterm/addon-web-links'
import 'xterm/css/xterm.css'

interface TerminalPanelProps {
  folderPath: string | null
  isVisible: boolean
}

const TerminalPanel: React.FC<TerminalPanelProps> = ({ folderPath, isVisible }) => {
  const terminalRef = useRef<HTMLDivElement>(null)
  const termRef = useRef<Terminal | null>(null)
  const fitAddonRef = useRef<FitAddon | null>(null)
  const initializedRef = useRef(false)
  const [error, setError] = useState<string | null>(null)

  const initTerminal = useCallback(async () => {
    if (!terminalRef.current || initializedRef.current) return

    // Check if electronAPI is available
    if (!window.electronAPI) {
      setError('Error: electronAPI not available')
      return
    }
    if (!window.electronAPI.terminal) {
      setError('Error: terminal API not available')
      return
    }

    try {
      const term = new Terminal({
        cursorBlink: true,
        fontFamily: "'Fira Code', 'Cascadia Code', 'Consolas', monospace",
        fontSize: 14,
        theme: {
          background: '#1e1e1e',
          foreground: '#cccccc',
          cursor: '#ffffff',
          selectionBackground: '#264f78',
          black: '#000000',
          red: '#cd3131',
          green: '#0dbc79',
          yellow: '#e5e510',
          blue: '#2472c8',
          magenta: '#bc3fbc',
          cyan: '#11a8cd',
          white: '#e5e5e5',
          brightBlack: '#666666',
          brightRed: '#f14c4c',
          brightGreen: '#23d18b',
          brightYellow: '#f5f543',
          brightBlue: '#3b8eea',
          brightMagenta: '#d670d6',
          brightCyan: '#29b8db',
          brightWhite: '#e5e5e5'
        }
      })

      const fitAddon = new FitAddon()
      term.loadAddon(fitAddon)
      term.loadAddon(new WebLinksAddon())

      term.open(terminalRef.current)
      fitAddon.fit()
      termRef.current = term
      fitAddonRef.current = fitAddon
      initializedRef.current = true

      term.write('Starting terminal...\r\n')

      // Pass folderPath or null, main process will handle fallback to HOME
      const cwd = folderPath || null
      term.write(`CWD: ${cwd || 'HOME'}\r\n`)
      
      // Call main process to start shell
      const result = await window.electronAPI.terminal.start(cwd)
      
      if (!result.success) {
        term.write(`Error starting terminal: ${result.error}\r\n`)
        setError(result.error)
        return
      }

      term.write('Terminal started.\r\n')

      window.electronAPI.terminal.onData((data) => {
        term.write(data)
      })

      window.electronAPI.terminal.onExit(() => {
        term.write('\r\n[Process exited]\r\n')
      })

      term.onData((data) => {
        window.electronAPI?.terminal.write(data)
      })

    } catch (err: any) {
      console.error('Terminal init error:', err)
      setError(err.message)
      if (termRef.current) {
        termRef.current.write(`\r\nError: ${err.message}\r\n`)
      }
    }
  }, [folderPath])

  useEffect(() => {
    if (isVisible && !initializedRef.current) {
      setTimeout(() => initTerminal(), 100)
    }
  }, [isVisible, initTerminal])

  useEffect(() => {
    if (isVisible && fitAddonRef.current) {
      setTimeout(() => fitAddonRef.current?.fit(), 50)
    }
  }, [isVisible])

  if (!isVisible) return null

  if (error) {
    return React.createElement('div', {
      style: { padding: '20px', color: '#f48771', fontFamily: 'monospace' }
    }, `Terminal Error: ${error}`)
  }

  return React.createElement('div', {
    ref: terminalRef,
    style: {
      width: '100%',
      height: '100%',
      background: '#1e1e1e'
    }
  })
}

export default TerminalPanel

import React, { useState } from 'react'
import Editor from '@monaco-editor/react'

interface EditorPanelProps {
  filePath: string | null
  fileName: string | null
  content: string
  language: string
  onChange: (value: string) => void
  showWelcome: boolean
}

const EditorPanel: React.FC<EditorPanelProps> = ({ filePath, fileName, content, language, onChange, showWelcome }) => {
  const [theme] = useState('vs-dark')

  if (showWelcome) {
    return React.createElement('div', {
      style: {
        flex: 1,
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        background: '#1e1e1e',
        color: '#858585',
        gap: '20px'
      }
    }, [
      React.createElement('h1', {
        key: 'title',
        style: {
          fontSize: '48px',
          fontWeight: 300,
          color: '#ffffff',
          margin: 0,
          letterSpacing: '-1px'
        }
      }, 'ViStudio'),
      React.createElement('p', {
        key: 'subtitle',
        style: { fontSize: '16px', margin: 0 }
      }, 'A modern, extensible code editor'),
      React.createElement('div', {
        key: 'shortcuts',
        style: { marginTop: '30px', textAlign: 'center' }
      }, [
        React.createElement('p', { key: 'p1', style: { margin: '8px 0', fontSize: '14px' } }, 'Open a file or folder to get started'),
        React.createElement('p', { key: 'p2', style: { margin: '8px 0', fontSize: '13px', fontFamily: 'monospace', color: '#007acc' } }, 'Ctrl+O - Open File'),
        React.createElement('p', { key: 'p3', style: { margin: '8px 0', fontSize: '13px', fontFamily: 'monospace', color: '#007acc' } }, 'Ctrl+Shift+O - Open Folder')
      ])
    ])
  }

  return React.createElement('div', {
    style: { flex: 1, display: 'flex', flexDirection: 'column', background: '#1e1e1e', overflow: 'hidden' }
  }, [
    React.createElement(Editor, {
      key: 'editor',
      height: '100%',
      theme,
      path: filePath || fileName || 'untitled',
      defaultLanguage: language,
      value: content,
      onChange: (value) => onChange(value || ''),
      options: {
        minimap: { enabled: true },
        fontSize: 14,
        fontFamily: "'Fira Code', 'Cascadia Code', 'Consolas', monospace",
        fontLigatures: true,
        automaticLayout: true,
        scrollBeyondLastLine: true,
        smoothScrolling: true,
        cursorBlinking: 'smooth',
        cursorSmoothCaretAnimation: 'on',
        renderWhitespace: 'selection',
        bracketPairColorization: { enabled: true },
        guides: { bracketPairs: true, indentation: true },
        tabSize: 2,
        wordWrap: 'off',
        lineNumbers: 'on',
        renderLineHighlight: 'all',
        scrollbar: {
          verticalScrollbarSize: 10,
          horizontalScrollbarSize: 10
        }
      }
    })
  ])
}

export default EditorPanel

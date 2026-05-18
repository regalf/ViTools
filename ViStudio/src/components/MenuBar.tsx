import React, { useState, useEffect, useRef } from 'react'

interface MenuItem {
  label: string
  action?: string
  shortcut?: string
  separator?: boolean
}

interface MenuBarProps {
  onAction: (action: string) => void
}

const menuTemplate: { label: string; items: MenuItem[] }[] = [
  {
    label: 'File',
    items: [
      { label: 'New File', action: 'menu:new-file', shortcut: 'Ctrl+N' },
      { label: 'New Project...', action: 'menu:new-project' },
      { separator: true },
      { label: 'Open File...', action: 'menu:open-file', shortcut: 'Ctrl+O' },
      { label: 'Open Folder...', action: 'menu:open-folder', shortcut: 'Ctrl+Shift+O' },
      { separator: true },
      { label: 'Save', action: 'menu:save', shortcut: 'Ctrl+S' },
      { label: 'Save As...', action: 'menu:save-as', shortcut: 'Ctrl+Shift+S' },
      { separator: true },
      { label: 'Exit', action: 'menu:exit' }
    ]
  },
  {
    label: 'Edit',
    items: [
      { label: 'Undo', action: 'menu:undo', shortcut: 'Ctrl+Z' },
      { label: 'Redo', action: 'menu:redo', shortcut: 'Ctrl+Y' },
      { separator: true },
      { label: 'Cut', action: 'menu:cut', shortcut: 'Ctrl+X' },
      { label: 'Copy', action: 'menu:copy', shortcut: 'Ctrl+C' },
      { label: 'Paste', action: 'menu:paste', shortcut: 'Ctrl+V' },
      { separator: true },
      { label: 'Find', action: 'menu:find', shortcut: 'Ctrl+F' },
      { label: 'Replace', action: 'menu:replace', shortcut: 'Ctrl+H' }
    ]
  },
  {
    label: 'View',
    items: [
      { label: 'Toggle Sidebar', action: 'menu:toggle-sidebar', shortcut: 'Ctrl+B' },
      { label: 'Toggle Terminal', action: 'menu:toggle-terminal', shortcut: 'Ctrl+`' },
      { label: 'Command Palette', action: 'menu:command-palette', shortcut: 'Ctrl+Shift+P' },
      { separator: true },
      { label: 'Zoom In', action: 'menu:zoom-in', shortcut: 'Ctrl+=' },
      { label: 'Zoom Out', action: 'menu:zoom-out', shortcut: 'Ctrl+-' },
      { label: 'Reset Zoom', action: 'menu:zoom-reset' }
    ]
  },
  {
    label: 'Help',
    items: [
      { label: 'About ViStudio', action: 'menu:about' }
    ]
  }
]

const MenuBar: React.FC<MenuBarProps> = ({ onAction }) => {
  const [activeMenu, setActiveMenu] = useState<number | null>(null)
  const containerRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    const handleClickOutside = (e: MouseEvent) => {
      if (containerRef.current && !containerRef.current.contains(e.target as Node)) {
        setActiveMenu(null)
      }
    }
    document.addEventListener('mousedown', handleClickOutside)
    return () => document.removeEventListener('mousedown', handleClickOutside)
  }, [])

  const handleMenuClick = (index: number) => {
    setActiveMenu(activeMenu === index ? null : index)
  }

  const handleItemClick = (action?: string) => {
    if (action) {
      onAction(action)
    }
    setActiveMenu(null)
  }

  return React.createElement('div', {
    ref: containerRef,
    style: {
      height: '30px',
      background: '#323233',
      display: 'flex',
      alignItems: 'center',
      padding: '0 8px',
      fontSize: '13px',
      color: '#cccccc',
      userSelect: 'none',
      borderBottom: '1px solid #2b2b2b'
    }
  }, [
    React.createElement('div', {
      key: 'logo',
      style: { marginRight: '12px', fontWeight: 600, color: '#007acc', fontSize: '14px' }
    }, 'ViStudio'),
    ...menuTemplate.map((menu, menuIndex) => {
      const isActive = activeMenu === menuIndex
      return React.createElement('div', {
        key: menu.label,
        style: { position: 'relative' }
      }, [
        React.createElement('div', {
          key: 'label',
          onClick: () => handleMenuClick(menuIndex),
          onMouseEnter: () => { if (activeMenu !== null) setActiveMenu(menuIndex) },
          style: {
            padding: '4px 10px',
            cursor: 'pointer',
            background: isActive ? '#505050' : 'transparent',
            borderRadius: '3px'
          }
        }, menu.label),
        isActive && React.createElement('div', {
          key: 'dropdown',
          style: {
            position: 'absolute',
            top: '100%',
            left: 0,
            background: '#3c3c3c',
            border: '1px solid #555',
            borderRadius: '4px',
            padding: '4px 0',
            minWidth: '220px',
            zIndex: 1000,
            boxShadow: '0 4px 12px rgba(0,0,0,0.5)'
          }
        }, menu.items.map((item, itemIndex) => {
          if (item.separator) {
            return React.createElement('div', {
              key: itemIndex,
              style: { height: '1px', background: '#555', margin: '4px 0' }
            })
          }
          return React.createElement('div', {
            key: itemIndex,
            onClick: () => handleItemClick(item.action),
            onMouseEnter: (e: React.MouseEvent<HTMLDivElement>) => {
              e.currentTarget.style.background = '#094771'
              e.currentTarget.style.color = '#ffffff'
            },
            onMouseLeave: (e: React.MouseEvent<HTMLDivElement>) => {
              e.currentTarget.style.background = 'transparent'
              e.currentTarget.style.color = '#cccccc'
            },
            style: {
              padding: '6px 24px',
              cursor: 'pointer',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              color: '#cccccc'
            }
          }, [
            React.createElement('span', { key: 'label' }, item.label),
            item.shortcut && React.createElement('span', {
              key: 'shortcut',
              style: { color: '#858585', fontSize: '12px', marginLeft: '30px' }
            }, item.shortcut)
          ])
        }))
      ])
    })
  ])
}

export default MenuBar

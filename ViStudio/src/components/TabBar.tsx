import React, { useState } from 'react'
import { EditorTab } from '../types'

interface TabBarProps {
  tabs: EditorTab[]
  activeTabId: string | null
  onSwitch: (id: string) => void
  onClose: (id: string) => void
  onReorder: (tabs: EditorTab[]) => void
}

const TabBar: React.FC<TabBarProps> = ({ tabs, activeTabId, onSwitch, onClose, onReorder }) => {
  const [draggedTabId, setDraggedTabId] = useState<string | null>(null)
  const [dragOverTabId, setDragOverTabId] = useState<string | null>(null)

  const handleDragStart = (e: React.DragEvent, tabId: string) => {
    setDraggedTabId(tabId)
    e.dataTransfer.effectAllowed = 'move'
  }

  const handleDragOver = (e: React.DragEvent, tabId: string) => {
    e.preventDefault()
    e.dataTransfer.dropEffect = 'move'
    setDragOverTabId(tabId)
  }

  const handleDrop = (e: React.DragEvent, targetTabId: string) => {
    e.preventDefault()
    setDragOverTabId(null)
    
    if (!draggedTabId || draggedTabId === targetTabId) return

    const draggedIndex = tabs.findIndex(t => t.id === draggedTabId)
    const targetIndex = tabs.findIndex(t => t.id === targetTabId)

    const newTabs = [...tabs]
    const [draggedTab] = newTabs.splice(draggedIndex, 1)
    newTabs.splice(targetIndex, 0, draggedTab)

    onReorder(newTabs)
    setDraggedTabId(null)
  }

  const handleDragEnd = () => {
    setDraggedTabId(null)
    setDragOverTabId(null)
  }

  return React.createElement('div', {
    style: {
      display: 'flex',
      background: '#2d2d2d',
      height: '35px',
      overflowX: 'auto',
      overflowY: 'hidden',
      borderBottom: '1px solid #1e1e1e'
    }
  }, tabs.map(tab => {
    const isActive = tab.id === activeTabId
    const isDragOver = dragOverTabId === tab.id
    const isDragging = draggedTabId === tab.id

    return React.createElement('div', {
      key: tab.id,
      onClick: () => onSwitch(tab.id),
      draggable: true,
      onDragStart: (e: React.DragEvent) => handleDragStart(e, tab.id),
      onDragOver: (e: React.DragEvent) => handleDragOver(e, tab.id),
      onDrop: (e: React.DragEvent) => handleDrop(e, tab.id),
      onDragEnd: handleDragEnd,
      style: {
        display: 'flex',
        alignItems: 'center',
        padding: '0 10px',
        background: isActive ? '#1e1e1e' : '#2d2d2d',
        color: isActive ? '#ffffff' : '#969696',
        fontSize: '13px',
        cursor: 'grab',
        borderRight: '1px solid #252526',
        minWidth: '120px',
        maxWidth: '200px',
        userSelect: 'none',
        position: 'relative',
        opacity: isDragging ? 0.5 : 1,
        borderLeft: isDragOver ? '2px solid #007acc' : 'none',
        marginLeft: isDragOver ? '-2px' : '0'
      }
    }, [
      React.createElement('span', {
        key: 'name',
        style: {
          flex: 1,
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap',
          marginRight: tab.isModified ? '4px' : '0'
        }
      }, tab.name),
      tab.isModified && React.createElement('span', {
        key: 'dot',
        style: {
          width: '8px',
          height: '8px',
          borderRadius: '50%',
          background: '#ffffff',
          marginRight: '6px'
        }
      }),
      React.createElement('span', {
        key: 'close',
        onClick: (e: any) => { e.stopPropagation(); onClose(tab.id) },
        style: {
          marginLeft: '8px',
          fontSize: '16px',
          lineHeight: '1',
          opacity: isActive ? 1 : 0,
          transition: 'opacity 0.2s',
          fontWeight: 'bold'
        }
      }, '×')
    ])
  }))
}

export default TabBar

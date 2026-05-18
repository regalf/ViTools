import React, { useState } from 'react'
import ProjectExplorer from './ProjectExplorer'

interface SidebarProps {
  isOpen: boolean
  folderPath: string | null
  onOpenFolder: () => void
  onFileClick: (path: string) => void
  onProjectLoad: (projectPath: string) => void
}

const Sidebar: React.FC<SidebarProps> = ({ isOpen, folderPath, onOpenFolder, onFileClick, onProjectLoad }) => {
  return React.createElement('div', {
    style: {
      width: isOpen ? '250px' : '0',
      background: '#252526',
      borderRight: '1px solid #3c3c3c',
      display: 'flex',
      flexDirection: 'column',
      overflow: 'hidden',
      transition: 'width 0.2s'
    }
  }, [
    React.createElement('div', {
      key: 'header',
      style: {
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        padding: '8px 12px',
        fontSize: '11px',
        fontWeight: 600,
        letterSpacing: '0.8px',
        color: '#858585',
        borderBottom: '1px solid #3c3c3c'
      }
    }, [
      React.createElement('span', { key: 'title' }, 'EXPLORER')
    ]),
    React.createElement('div', {
      key: 'actions',
      style: {
        padding: '8px 12px',
        borderBottom: '1px solid #3c3c3c'
      }
    }, [
      React.createElement('button', {
        key: 'open-folder',
        onClick: onOpenFolder,
        title: 'Open Folder',
        style: {
          width: '100%',
          padding: '4px 8px',
          background: '#0e639c',
          color: 'white',
          border: 'none',
          borderRadius: '3px',
          cursor: 'pointer',
          fontSize: '12px'
        }
      }, 'Open Folder')
    ]),
    React.createElement('div', {
      key: 'content',
      style: { flex: 1, overflowY: 'auto' }
    }, React.createElement(ProjectExplorer, {
      folderPath,
      onFileClick,
      onProjectLoad
    }))
  ])
}

export default Sidebar

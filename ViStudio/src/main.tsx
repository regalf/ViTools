import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'

console.log('main.tsx loaded')

const rootElement = document.getElementById('root')
console.log('root element:', rootElement)

if (rootElement) {
  console.log('Creating React root...')
  const root = ReactDOM.createRoot(rootElement)
  console.log('Rendering App...')
  root.render(
    React.createElement(React.StrictMode, null,
      React.createElement(App)
    )
  )
  console.log('App rendered!')
} else {
  console.error('Root element not found!')
}

import {defineConfig} from 'vite';
import react from '@vitejs/plugin-react';
import {resolve} from 'path';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],

  // Library build mode - we use renderToString() directly
  build: {
    lib: {
      entry: resolve(__dirname, 'src/utils/renderToHtml.ts'),
      name: 'reportTools',
      fileName: 'report-tools',
      formats: ['es'],
    },
    rollupOptions: {
      external: ['react', 'react-dom', 'd3'],
    },
  },

  // Path aliases
  resolve: {
    alias: {
      '@': resolve(__dirname, 'src'),
      '@components': resolve(__dirname, 'src/components'),
      '@algorithms': resolve(__dirname, 'src/algorithms'),
      '@utils': resolve(__dirname, 'src/utils'),
      '@types': resolve(__dirname, 'src/types'),
      '@styles': resolve(__dirname, 'src/styles'),
    },
  },

  // Optimize deps
  optimizeDeps: {
    include: ['react', 'react-dom', 'd3'],
  },
});

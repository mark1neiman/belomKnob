import React from 'react';
import { ChevronLeft, ChevronRight, Circle } from 'lucide-react';

interface VirtualEncoderProps {
  onRotate: (direction: 1 | -1) => void;
  onPressDown: () => void;
  onPressUp: () => void;
  isPressed: boolean;
}

export function VirtualEncoder({ onRotate, onPressDown, onPressUp, isPressed }: VirtualEncoderProps) {
  return (
    <div className="fixed bottom-8 right-8 z-50 flex flex-col items-center gap-2 p-4 bg-zinc-900/90 backdrop-blur-md rounded-2xl border border-zinc-700 shadow-2xl">
      <div className="text-[10px] uppercase tracking-widest text-zinc-500 font-bold mb-1">Encoder Sim</div>
      
      <div className="flex items-center gap-4">
        <button 
          onClick={() => onRotate(-1)}
          className="p-3 rounded-full bg-zinc-800 hover:bg-zinc-700 active:bg-zinc-600 text-zinc-400 hover:text-white transition-colors border border-zinc-700"
          title="Rotate Left (Arrow Left)"
        >
          <ChevronLeft size={20} />
        </button>
        
        <button 
          onMouseDown={onPressDown}
          onMouseUp={onPressUp}
          onMouseLeave={onPressUp}
          onTouchStart={onPressDown}
          onTouchEnd={onPressUp}
          className={`w-16 h-16 rounded-full border-4 flex items-center justify-center transition-all duration-100 ${
            isPressed 
              ? 'bg-[#E10600] border-[#E10600] scale-95 shadow-inner' 
              : 'bg-zinc-800 border-zinc-600 hover:border-zinc-500 hover:bg-zinc-700 shadow-lg'
          }`}
          title="Press (Enter/Space)"
        >
          <div className={`w-3 h-3 rounded-full ${isPressed ? 'bg-white' : 'bg-zinc-400'}`} />
        </button>

        <button 
          onClick={() => onRotate(1)}
          className="p-3 rounded-full bg-zinc-800 hover:bg-zinc-700 active:bg-zinc-600 text-zinc-400 hover:text-white transition-colors border border-zinc-700"
          title="Rotate Right (Arrow Right)"
        >
          <ChevronRight size={20} />
        </button>
      </div>
      
      <div className="text-[9px] text-zinc-600 font-mono mt-1">
        Hold center for Menu (3s)
      </div>
    </div>
  );
}

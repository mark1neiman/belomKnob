import React from 'react';
import { Wind, Fan } from 'lucide-react';
import { motion } from 'motion/react';

export function PurificationScreen() {
  return (
    <div className="w-full h-full bg-black flex flex-col items-center justify-center text-white relative overflow-hidden">
      
      {/* Background Pulse */}
      <motion.div 
        animate={{ opacity: [0.1, 0.3, 0.1] }}
        transition={{ duration: 4, repeat: Infinity, ease: "easeInOut" }}
        className="absolute inset-0 bg-emerald-900/20 rounded-full blur-3xl scale-150"
      />

      <div className="z-10 flex flex-col items-center gap-6">
        <div className="relative">
             {/* Rotating Fan Icon */}
            <motion.div
                animate={{ rotate: 360 }}
                transition={{ duration: 3, repeat: Infinity, ease: "linear" }}
            >
                <Fan size={64} className="text-emerald-500" strokeWidth={1.5} />
            </motion.div>
            
            {/* Wind particles overlay */}
            <div className="absolute top-0 right-0 -mt-2 -mr-2">
                 <motion.div
                    animate={{ x: [0, 5, 0], y: [0, -5, 0], opacity: [0.5, 1, 0.5] }}
                    transition={{ duration: 2, repeat: Infinity }}
                 >
                    <Wind size={24} className="text-emerald-300/50" />
                 </motion.div>
            </div>
        </div>

        <div className="flex flex-col items-center gap-2">
            <h2 className="text-2xl font-bold tracking-widest text-emerald-500 uppercase">
                Purifying
            </h2>
            <div className="flex gap-1">
                <motion.div 
                    animate={{ opacity: [0, 1, 0] }} 
                    transition={{ duration: 1.5, repeat: Infinity, delay: 0 }}
                    className="w-1.5 h-1.5 bg-emerald-500 rounded-full" 
                />
                <motion.div 
                    animate={{ opacity: [0, 1, 0] }} 
                    transition={{ duration: 1.5, repeat: Infinity, delay: 0.3 }}
                    className="w-1.5 h-1.5 bg-emerald-500 rounded-full" 
                />
                <motion.div 
                    animate={{ opacity: [0, 1, 0] }} 
                    transition={{ duration: 1.5, repeat: Infinity, delay: 0.6 }}
                    className="w-1.5 h-1.5 bg-emerald-500 rounded-full" 
                />
            </div>
        </div>

        <p className="text-zinc-500 text-xs font-mono uppercase tracking-widest mt-4">
            Press to Wake
        </p>
      </div>
    </div>
  );
}

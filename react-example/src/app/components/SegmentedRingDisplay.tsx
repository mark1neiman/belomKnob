import React, { useEffect, useState } from 'react';
import { ShieldCheck, ShieldAlert, Lock, Unlock } from 'lucide-react';
import logoLight from "figma:asset/3b0dc3d3c8d631fd14d78bf42d7083aee43bd908.png"; // Black text (for light theme)
import logoDark from "figma:asset/35250e797be50b1147f11e22eccfe73e83a4ee66.png"; // White text (for dark theme)

interface DisplayProps {
  speed: number;
  theme: 'dark' | 'light';
  filterStatus?: 'good' | 'warning' | 'critical';
  isLocked?: boolean;
  isBooting?: boolean;
  accentColor?: string;
}

// Helper to adjust color brightness
function adjustColor(hex: string, amount: number) {
    return '#' + hex.replace(/^#/, '').replace(/../g, color => ('0' + Math.min(255, Math.max(0, parseInt(color, 16) + amount)).toString(16)).substr(-2));
}

export function SegmentedRingDisplay({ 
  speed, 
  theme, 
  filterStatus = 'good', 
  isLocked = false, 
  isBooting = false,
  accentColor = '#E10600'
}: DisplayProps) {
  const isDark = theme === 'dark';
  const brandColor = accentColor;
  // Automatically generate a lighter/secondary shade for effects
  // We'll lighten it by 40 units for the "pulse" peak
  // Or if it's already very light, we darken it.
  // Simple heuristic: just assume standard mid-tone colors and lighten.
  const secondaryColor = adjustColor(brandColor, 40); 
  
  const brandYellow = '#EAB308'; // Amber-500
  const brandGreen = '#10B981'; // Emerald-500
  const fixedRed = '#EF4444'; // Red-500 (Always red for Critical/Overdrive)
  
  // Create 40 segments
  const totalSegments = 40;
  
  // --- Boot Sequence Animation State ---
  const [bootProgress, setBootProgress] = useState(0);

  useEffect(() => {
    if (isBooting) {
        let startTime = Date.now();
        const duration = 2000; // 2 seconds per fill
        const pause = 500; // 0.5 second pause at full
        
        const timer = setInterval(() => {
            const now = Date.now();
            const cycleTime = duration + pause;
            const elapsed = (now - startTime) % cycleTime;
            
            if (elapsed < duration) {
                // Filling phase
                const progress = elapsed / duration;
                setBootProgress(progress * 100);
            } else {
                // Pause phase (keep at 100%)
                setBootProgress(100);
            }
        }, 16);
        
        return () => clearInterval(timer);
    } else {
        setBootProgress(0);
    }
  }, [isBooting]);

  // Calculate active segments
  // If booting, use bootProgress (0-100). If normal, use speed.
  const displayValue = isBooting ? bootProgress : speed;
  const activeSegmentsCount = Math.round((displayValue / 100) * totalSegments);

  // Determine Filter Color
  const filterColor = 
    filterStatus === 'critical' ? fixedRed :
    filterStatus === 'warning' ? brandYellow :
    brandGreen;

  return (
    <div className="relative w-full h-full flex items-center justify-center">
      {/* Custom Animation Style */}
      <style>{`
        @keyframes slow-blink {
          0%, 100% { opacity: 1; transform: scale(1); box-shadow: 0 0 8px var(--blink-shadow); }
          50% { opacity: 0.4; transform: scale(0.95); box-shadow: 0 0 2px var(--blink-shadow-dim); }
        }
        .animate-last-segment {
          animation: slow-blink 2s ease-in-out infinite;
        }
        @keyframes fade-in {
            0% { opacity: 0; transform: translateY(10px); }
            100% { opacity: 1; transform: translateY(0); }
        }
        .animate-fade-in {
            animation: fade-in 0.8s ease-out forwards;
        }
        @keyframes ember-pulse {
            0%, 70%, 100% { background-color: ${brandColor}; }
            50% { background-color: ${secondaryColor}; }
        }
        .animate-ember {
            animation-name: ember-pulse;
            animation-timing-function: ease-in-out;
            animation-iteration-count: infinite;
        }
      `}</style>

      <div className={`w-[320px] h-[320px] rounded-full relative flex items-center justify-center transition-colors duration-500 ${
        isDark 
            ? 'bg-zinc-950 border border-zinc-900 shadow-xl shadow-black/40' 
            : 'bg-white border border-zinc-200 shadow-lg shadow-zinc-200/50'
      }`}>
         {/* Segments Container */}
         <div className="absolute inset-0 z-0">
           {Array.from({ length: totalSegments }).map((_, i) => {
             const angle = (i / totalSegments) * 360;
             const isActive = i < activeSegmentsCount;
             
             // Identify the last active segment to animate
             // Only active in normal mode. During boot, we just fill up solidly.
             const isLastActive = !isBooting && isActive && i === activeSegmentsCount - 1 && speed < 100 && speed > 0;
             
             // Boot Color: During boot, maybe use Green/White to indicate "System Check"?
             // Or stick to Brand Red for "Powering Up"? Let's stick to Brand Red for consistency.
             // If booting, let's make the inactive segments slightly visible to show the "track"?
             
             const segmentColor = isActive 
                ? brandColor 
                : (isDark ? '#52525b' : '#a1a1aa');

             // Apply "Smoldering" animation to active segments (except the last blinking one)
             const isEmber = isActive && !isLastActive && !isBooting;
             
             // Generate "Stable Random" numbers based on index 'i'
             // This prevents flickering on re-renders while looking random
             const randomDelay = -((i * 1337) % 4000) / 1000; // 0s to -4s delay
             const randomDuration = 2 + ((i * 997) % 2000) / 1000; // 2s to 4s duration

             return (
               <div
                 key={i}
                 className="absolute top-0 left-1/2 -translate-x-1/2"
                 style={{ 
                    height: '160px', // Radius
                    transformOrigin: 'bottom center',
                    transform: `rotate(${angle}deg)` 
                 }}
               >
                 {/* The segment itself - pushed to the top (outer edge) */}
                 <div
                   className={`w-1.5 h-5 rounded-sm transition-all duration-300 ${
                     isLastActive ? 'animate-last-segment' : ''
                   } ${
                     isEmber ? 'animate-ember' : ''
                   } ${
                     isActive
                       ? 'opacity-100 scale-100'
                       : 'opacity-20 scale-90'
                   }`}
                   style={{
                     marginTop: '16px', // Offset from edge
                     backgroundColor: segmentColor,
                     
                     // Apply the random timing inline
                     animationDelay: isEmber ? `${randomDelay}s` : undefined,
                     animationDuration: isEmber ? `${randomDuration}s` : undefined,
                     
                     // Static shadow for active segments in dark mode (except the animating one)
                     boxShadow: (isActive && isDark && !isLastActive) ? `0 0 8px ${brandColor}66` : 'none',
                     
                     // CSS Variables for the animation
                     // @ts-ignore
                     '--blink-shadow': isDark ? `${brandColor}66` : 'transparent',
                     '--blink-shadow-dim': isDark ? `${brandColor}33` : 'transparent',
                   } as React.CSSProperties}
                 />
               </div>
             );
           })}
        </div>

        {/* Center Interface */}
        <div className="absolute inset-0 z-10 flex flex-col items-center justify-center pointer-events-none">
            
            {isBooting ? (
                // --- BOOT SCREEN ---
                <div 
                    className="animate-fade-in w-full h-full flex flex-col items-center justify-center translate-y-4"
                >
                    {/* Logo */}
                    <img   
                        src={isDark ? logoDark : logoLight} 
                        alt="Logo" 
                        className="w-52 h-auto object-contain opacity-100 mb-2"
                    />

                    {/* System Check Status */}
                    <div className="flex flex-col items-center justify-center gap-3">
                        <div className={`text-[10px] font-bold font-mono uppercase tracking-[0.2em] animate-pulse ${isDark ? 'text-zinc-500' : 'text-zinc-400'}`}>
                            BELOM A2
                        </div>
                        {/* Progress Bar Line */}
                        <div className={`h-0.5 w-12 rounded-full overflow-hidden ${isDark ? 'bg-zinc-800' : 'bg-zinc-300'}`}>
                            <div 
                                className="h-full transition-all duration-75 ease-linear"
                                style={{ width: `${bootProgress}%`, backgroundColor: brandColor }}
                            />
                        </div>
                    </div>
                </div>
            ) : (
                // --- NORMAL INTERFACE ---
                <div className="animate-fade-in flex flex-col items-center justify-center relative">
                    {/* Overdrive Warning - Absolute to avoid layout shift */}
                    <div 
                        className={`absolute -top-4 text-[10px] font-bold uppercase tracking-widest whitespace-nowrap transition-opacity duration-300 ${speed > 100 ? 'opacity-100' : 'opacity-0'}`}
                        style={{ color: fixedRed }}
                    >
                        Overdrive Mode
                    </div>
                    
                    {/* Speed Value - Dominant, Vertically Centered */}
                    <div className={`text-7xl font-bold tracking-tighter leading-none transition-colors duration-300 tabular-nums ${
                        isDark ? 'text-zinc-50' : 'text-zinc-900'
                    }`} style={{ letterSpacing: '-0.06em', marginBottom: '0.1em' }}>
                        {speed}
                    </div>

                    {/* Status Row (Filter & Lock) - Small & Compact */}
                    <div className="flex items-center gap-4 mt-4">
                        
                        {/* Lock Status */}
                        <div className={`flex items-center gap-1.5 ${isLocked ? '' : (isDark ? 'text-zinc-500' : 'text-zinc-500')}`} style={{ color: isLocked ? brandColor : undefined }}>
                            {isLocked ? <Lock size={14} strokeWidth={2.5} /> : <Unlock size={14} strokeWidth={2.5} />}
                            <span className="text-[9px] font-bold uppercase tracking-widest hidden sm:block pt-0.5">
                                {isLocked ? 'Locked' : 'Unlock'}
                            </span>
                        </div>

                        {/* Vertical Divider */}
                        <div className={`w-px h-3 ${isDark ? 'bg-zinc-800' : 'bg-zinc-300'}`} />

                        {/* Filter Status */}
                        <div className="flex items-center gap-1.5" style={{ color: filterColor }}>
                            {filterStatus === 'good' ? (
                                <ShieldCheck size={14} strokeWidth={2.5} />
                            ) : (
                                <ShieldAlert size={14} strokeWidth={2.5} />
                            )}
                            <span className="text-[9px] font-bold uppercase tracking-widest hidden sm:block pt-0.5">
                                Filter
                            </span>
                        </div>
                    </div>
                </div>
            )}


        </div>
      </div>
    </div>
  );
}

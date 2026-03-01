import { useState, useEffect, useRef } from 'react';
import { SegmentedRingDisplay } from './components/SegmentedRingDisplay';
import { SettingsScreen, SettingsState } from './components/SettingsScreen';
import { PurificationScreen } from './components/PurificationScreen';
import { VirtualEncoder } from './components/VirtualEncoder';
import { useEncoder } from './hooks/useEncoder';
import { ShieldCheck, ShieldAlert, Lock, Unlock, Settings2, LayoutTemplate, Power } from 'lucide-react';

export default function App() {
  const [speed, setSpeed] = useState(50);
  const [filterStatus, setFilterStatus] = useState<'good' | 'warning' | 'critical'>('good');
  const [isLocked, setIsLocked] = useState(false);
  const [activeTab, setActiveTab] = useState<'segmented' | 'welcome' | 'settings'>('segmented');
  const [isPurifying, setIsPurifying] = useState(false);
  const lastPressTime = useRef(0);
  
  // Settings View State (Lifted up from SettingsScreen)
  const [settingsView, setSettingsView] = useState<string>('root');

  // Encoder Hook
  const { 
    rotateEvent, 
    isPressed, 
    pressCount, 
    longPressCount,
    manualRotate, 
    manualPressDown, 
    manualPressUp 
  } = useEncoder();

  // Settings State
  const [settings, setSettings] = useState<SettingsState>({
    language: 'en',
    brightness: 80,
    theme: 'dark', 
    rotation: 0,
    lockMode: true,
    filterLifetime: 100,
    filterUsed: 12,
    sensitivity: 15, // Default 15 clicks for 100%
    accentColor: '#E10600', // Default Red
    allowOverdrive: false // Default disabled
  });

  const [isScreenSleep, setIsScreenSleep] = useState(false);
  const lastInteraction = useRef(Date.now());

  // We determine booting state based on the active tab
  const isBooting = activeTab === 'welcome';
  const showSettings = activeTab === 'settings';

  // --- Idle Timer for Screen Sleep (30 mins) ---
  useEffect(() => {
    const checkInterval = setInterval(() => {
      const now = Date.now();
      // 30 minutes = 30 * 60 * 1000 = 1800000 ms
      if (now - lastInteraction.current > 1800000 && !isScreenSleep) {
        setIsScreenSleep(true);
      }
    }, 10000); // Check every 10 seconds

    return () => clearInterval(checkInterval);
  }, [isScreenSleep]);

  const updateSettings = (updates: Partial<SettingsState>) => {
    setSettings(prev => ({ ...prev, ...updates }));
    
    // If lock mode is disabled via settings, unlock the device
    if (updates.lockMode === false) {
        setIsLocked(false);
    }
  };

  // --- Encoder Logic ---

  // Track interaction to reset sleep timer
  useEffect(() => {
    if (rotateEvent !== 0 || pressCount > 0 || longPressCount > 0) {
      lastInteraction.current = Date.now();
    }
  }, [rotateEvent, pressCount, longPressCount]);

  // Wake up on press
  useEffect(() => {
    if (pressCount > 0 && isScreenSleep) {
      setIsScreenSleep(false);
    }
  }, [pressCount, isScreenSleep]);

  // Double Press Logic to Exit Purification Mode
  useEffect(() => {
    if (pressCount > 0) {
      if (isScreenSleep) return; // Ignore if sleeping (handled by wake up effect)

      const now = Date.now();
      const timeDiff = now - lastPressTime.current;
      
      if (isPurifying) {
        if (timeDiff < 500 && timeDiff > 50) { // Double click detected (debounce 50ms)
          setIsPurifying(false);
          // Restore to 0 or previous speed.
          setSpeed(0);
          setSettingsView('root'); // Ensure settings are reset
          setActiveTab('segmented');
        }
      }
      
      lastPressTime.current = now;
    }
  }, [pressCount, isPurifying, isScreenSleep]);

  // Long Press -> Navigation Logic
  useEffect(() => {
    if (longPressCount > 0 && !isPurifying && !isScreenSleep) {
        if (activeTab === 'settings') {
            // If inside a sub-menu, go back to root settings
            if (settingsView === 'filterLifetime' || settingsView === 'filterReset') {
                // If in filter submenu items, go back to filter menu
                setSettingsView('filter');
            } else if (settingsView === 'overdrive') {
                // If in overdrive settings, go back to root
                setSettingsView('root');
            } else if (settingsView !== 'root') {
                setSettingsView('root');
            } else {
                // If at root settings, exit to main screen
                setActiveTab('segmented');
            }
        } else {
            // If not in settings, go to settings (and ensure we start at root)
            setSettingsView('root');
            setActiveTab('settings');
        }
    }
  }, [longPressCount, isScreenSleep, activeTab, settingsView, isPurifying]); 

  // Rotate -> Adjust Speed (Main Screen)
  useEffect(() => {
    if (rotateEvent !== 0 && activeTab === 'segmented' && !isBooting && !isLocked && !isPurifying && !isScreenSleep) {
        // Use user-defined sensitivity (clicks for full range)
        // Allow up to 110% if filter is critical and overdrive is enabled
        const maxSpeed = (filterStatus === 'critical' && settings.allowOverdrive) ? 110 : 100;
        const range = maxSpeed;
        const clicksForFullRange = settings.sensitivity; 
        const stepSize = range / clicksForFullRange; 
        
        setSpeed(prev => {
            const next = prev + (rotateEvent * stepSize);
            return Math.max(0, Math.min(maxSpeed, Math.round(next)));
        });
    }
  }, [rotateEvent, activeTab, isBooting, isLocked, settings.sensitivity, filterStatus, settings.allowOverdrive, isScreenSleep]);

  // Short Press -> Toggle Lock (Main Screen)
  useEffect(() => {
    if (pressCount > 0 && activeTab === 'segmented' && !isBooting && !isPurifying && !isScreenSleep) {
        if (settings.lockMode) {
             setIsLocked(prev => !prev);
        }
    }
  }, [pressCount, activeTab, isBooting, settings.lockMode, isPurifying, isScreenSleep]);

  const handleStartPurification = () => {
    // 10% speed
    setSpeed(10);
    // Enter purification mode
    setIsPurifying(true);
    // Exit settings (so when we exit purification, we are back to main)
    setActiveTab('segmented');
    setSettingsView('root');
  };

  return (
    <div className="min-h-screen bg-zinc-950 text-zinc-100 font-sans flex overflow-hidden relative">
      
      {/* Virtual Encoder UI Overlay */}
      <VirtualEncoder 
        onRotate={manualRotate} 
        onPressDown={manualPressDown} 
        onPressUp={manualPressUp} 
        isPressed={isPressed} 
      />

      {/* Sidebar */}
      <aside className="w-80 flex-shrink-0 border-r border-zinc-800 bg-zinc-900/50 flex flex-col h-screen overflow-y-auto">
        
        {/* Sidebar Header */}
        <div className="p-6 border-b border-zinc-800">
          <div className="flex items-center gap-2 mb-1" style={{ color: settings.accentColor }}>
            <Settings2 size={20} />
            <span className="text-xs font-bold uppercase tracking-widest">Control Panel</span>
          </div>
          <h1 className="text-xl font-light tracking-tight text-white">Smart Knob</h1>
        </div>

        {/* Controls Container */}
        <div className="p-6 space-y-8 flex-1">
          
          {/* Speed Control */}
          <section>
            <div className="flex items-center justify-between mb-4">
                <label className="text-xs font-bold uppercase tracking-widest text-zinc-500">
                  Fan Speed
                </label>
                <span className="font-mono text-sm font-bold tabular-nums" style={{ color: settings.accentColor }}>
                    {speed}%
                </span>
            </div>
            
            <div className="relative h-48 flex items-center justify-center mb-6">
                 {/* Vertical Slider Construction */}
                 <div className="relative w-12 h-full bg-zinc-800 rounded-full overflow-hidden border border-zinc-700">
                    {/* Normal range (0-100) */}
                    <div 
                        className="absolute bottom-0 left-0 right-0 transition-all duration-100 ease-out"
                        style={{ 
                          height: `${Math.min(speed, 100)}%`, 
                          backgroundColor: settings.accentColor 
                        }}
                    />
                    {/* Overdrive range (100-110) */}
                    {speed > 100 && (
                      <div 
                          className="absolute bottom-full left-0 right-0 transition-all duration-100 ease-out"
                          style={{ 
                            height: `${((speed - 100) / 10) * 100}%`, 
                            backgroundColor: settings.accentColor,
                            opacity: 0.7,
                            boxShadow: `0 0 15px ${settings.accentColor}`
                          }}
                      />
                    )}
                    <input
                        type="range"
                        min="0"
                        max={(filterStatus === 'critical' && settings.allowOverdrive) ? "110" : "100"}
                        orient="vertical"
                        value={speed}
                        onChange={(e) => setSpeed(Number(e.target.value))}
                        disabled={isLocked}
                        className={`absolute inset-0 w-full h-full opacity-0 ${isLocked ? 'cursor-not-allowed' : 'cursor-pointer'} appearance-none z-10`}
                        style={{ WebkitAppearance: 'slider-vertical' } as any}
                    />
                 </div>
            </div>

            <div className="grid grid-cols-2 gap-2">
                 <button onClick={() => setSpeed(0)} disabled={isLocked} className="px-3 py-2 text-xs font-bold uppercase bg-zinc-800 text-zinc-400 rounded hover:bg-zinc-700 hover:text-white transition-colors border border-zinc-700">Stop</button>
                 <button 
                   onClick={() => setSpeed((filterStatus === 'critical' && settings.allowOverdrive) ? 110 : 100)} 
                   disabled={isLocked} 
                   className="px-3 py-2 text-xs font-bold uppercase bg-zinc-800 text-zinc-400 rounded hover:bg-zinc-700 hover:text-white transition-colors border border-zinc-700"
                 >
                   Max {(filterStatus === 'critical' && settings.allowOverdrive) && '+'}
                 </button>
            </div>
          </section>

          <div className="w-full h-px bg-zinc-800" />

          {/* Status Controls */}
          <section className="space-y-6">
             {/* Filter Status */}
             <div>
                <label className="block text-xs font-bold uppercase tracking-widest mb-3 text-zinc-500">
                    Filter Status
                </label>
                <div className="space-y-2">
                    <button 
                        onClick={() => setFilterStatus('good')}
                        className={`w-full flex items-center justify-between px-3 py-2 text-xs font-bold uppercase rounded border transition-colors ${filterStatus === 'good' ? 'bg-emerald-900/20 text-emerald-400 border-emerald-800' : 'bg-zinc-900/50 border-zinc-800 text-zinc-500 hover:border-zinc-700'}`}
                    >
                        <span className="flex items-center gap-2"><ShieldCheck size={14} /> Good</span>
                        {filterStatus === 'good' && <div className="w-1.5 h-1.5 rounded-full bg-emerald-400 shadow-[0_0_8px_rgba(52,211,153,0.5)]" />}
                    </button>
                    <button 
                        onClick={() => setFilterStatus('warning')}
                        className={`w-full flex items-center justify-between px-3 py-2 text-xs font-bold uppercase rounded border transition-colors ${filterStatus === 'warning' ? 'bg-amber-900/20 text-amber-400 border-amber-800' : 'bg-zinc-900/50 border-zinc-800 text-zinc-500 hover:border-zinc-700'}`}
                    >
                        <span className="flex items-center gap-2"><ShieldAlert size={14} /> Low</span>
                        {filterStatus === 'warning' && <div className="w-1.5 h-1.5 rounded-full bg-amber-400 shadow-[0_0_8px_rgba(251,191,36,0.5)]" />}
                    </button>
                    <button 
                        onClick={() => setFilterStatus('critical')}
                        className={`w-full flex items-center justify-between px-3 py-2 text-xs font-bold uppercase rounded border transition-colors ${filterStatus === 'critical' ? 'bg-red-900/20 text-red-400 border-red-800' : 'bg-zinc-900/50 border-zinc-800 text-zinc-500 hover:border-zinc-700'}`}
                    >
                        <span className="flex items-center gap-2"><ShieldAlert size={14} /> Replace</span>
                        {filterStatus === 'critical' && <div className="w-1.5 h-1.5 rounded-full bg-red-500 shadow-[0_0_8px_rgba(239,68,68,0.5)]" />}
                    </button>
                </div>
             </div>

             {/* Lock Status */}
             <div>
                <label className="block text-xs font-bold uppercase tracking-widest mb-3 text-zinc-500">
                    Security
                </label>
                <button 
                    onClick={() => {
                        if (!settings.lockMode) return;
                        setIsLocked(!isLocked)
                    }}
                    disabled={!settings.lockMode}
                    className={`w-full flex items-center justify-center gap-2 px-3 py-3 text-xs font-bold uppercase rounded border transition-all ${
                        !settings.lockMode
                            ? 'opacity-50 cursor-not-allowed bg-zinc-800/50 border-zinc-800 text-zinc-600'
                            : isLocked 
                                ? 'bg-zinc-100 text-zinc-900 border-zinc-100 shadow-[0_0_15px_rgba(255,255,255,0.1)]' 
                                : 'bg-zinc-900 border-zinc-800 text-zinc-500 hover:border-zinc-700'
                    }`}
                >
                    {isLocked ? <Lock size={14} /> : <Unlock size={14} />}
                    {isLocked ? 'Device Locked' : 'Unlocked'}
                </button>
             </div>
          </section>
        </div>

        {/* Footer info */}
        <div className="p-6 border-t border-zinc-800 text-[10px] text-zinc-600 font-mono">
            V 2.4.0 — BUILD 892<br/>
            INDUSTRIAL UI SYSTEM
        </div>

      </aside>

      {/* Main Content Area */}
      <main className="flex-1 h-screen overflow-y-auto bg-zinc-950 relative">
        {/* Background Grid Pattern */}
        <div className="absolute inset-0 bg-[linear-gradient(rgba(255,255,255,0.03)_1px,transparent_1px),linear-gradient(90deg,rgba(255,255,255,0.03)_1px,transparent_1px)] bg-[size:40px_40px] pointer-events-none" />
        
        <div className="relative p-12 max-w-6xl mx-auto min-h-full flex flex-col justify-center">
            
            {/* View Tabs */}
            <div className="flex justify-center mb-16">
                <div className="bg-zinc-900/80 backdrop-blur-md p-1 rounded-full border border-zinc-800 inline-flex">
                    <button 
                        onClick={() => setActiveTab('segmented')}
                        className={`flex items-center gap-2 px-6 py-2 rounded-full text-xs font-bold uppercase tracking-wider transition-all ${activeTab === 'segmented' ? 'bg-zinc-800 text-white shadow-lg' : 'text-zinc-500 hover:text-zinc-300'}`}
                    >
                        <LayoutTemplate size={14} /> Main
                    </button>
                    <button 
                        onClick={() => setActiveTab('welcome')}
                        className={`flex items-center gap-2 px-6 py-2 rounded-full text-xs font-bold uppercase tracking-wider transition-all ${activeTab === 'welcome' ? 'bg-[#E10600] text-white shadow-lg shadow-red-900/50' : 'text-zinc-500 hover:text-zinc-300'}`}
                    >
                        <Power size={14} /> Boot
                    </button>
                    <button 
                        onClick={() => setActiveTab('settings')}
                        className={`flex items-center gap-2 px-6 py-2 rounded-full text-xs font-bold uppercase tracking-wider transition-all ${activeTab === 'settings' ? 'bg-zinc-100 text-zinc-900 shadow-lg' : 'text-zinc-500 hover:text-zinc-300'}`}
                    >
                        <Settings2 size={14} /> Settings
                    </button>
                </div>
            </div>

            {/* Content Display */}
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-16 items-center">
                
                {/* Dark Mode Preview */}
                <div className="flex flex-col items-center gap-6">
                    <div className="text-xs font-mono uppercase tracking-widest text-zinc-500">Dark Environment</div>
                    <div 
                        className="relative group transition-transform duration-500 ease-in-out"
                        style={{ transform: `rotate(${settings.rotation}deg)` }}
                    >
                        {isPurifying ? (
                            <div className="w-[320px] h-[320px] rounded-full overflow-hidden shadow-2xl bg-black relative z-10 flex items-center justify-center">
                                {isScreenSleep ? (
                                    <div className="w-full h-full bg-black" />
                                ) : (
                                    <PurificationScreen />
                                )}
                            </div>
                        ) : showSettings ? (
                            <div className="w-[320px] h-[320px] rounded-full overflow-hidden shadow-2xl bg-black relative z-10">
                                {isScreenSleep ? (
                                    <div className="w-full h-full bg-black" />
                                ) : (
                                    <SettingsScreen 
                                        settings={settings}
                                        updateSettings={updateSettings}
                                        currentView={settingsView}
                                        onViewChange={setSettingsView}
                                        onStartPurification={handleStartPurification}
                                        theme="dark" // Force Dark
                                        encoderRotateEvent={rotateEvent}
                                        encoderPressCount={pressCount}
                                    />
                                )}
                            </div>
                        ) : (
                            <div className="w-[320px] h-[320px] rounded-full overflow-hidden shadow-2xl relative z-10">
                                 {isScreenSleep ? (
                                    <div className="w-full h-full bg-black" />
                                 ) : (
                                    <SegmentedRingDisplay 
                                        speed={speed} 
                                        theme="dark" 
                                        filterStatus={filterStatus} 
                                        isLocked={isLocked} 
                                        isBooting={isBooting}
                                        accentColor={settings.accentColor}
                                    />
                                 )}
                            </div>
                        )}
                        {/* Reflection/Glare effect for realism */}
                        <div className="absolute inset-0 rounded-full bg-gradient-to-tr from-white/0 via-white/5 to-transparent pointer-events-none opacity-50" />
                    </div>
                </div>

                {/* Light Mode Preview */}
                <div className="flex flex-col items-center gap-6">
                    <div className="text-xs font-mono uppercase tracking-widest text-zinc-500">Light Environment</div>
                    <div className="p-12 bg-zinc-200 rounded-[3rem] shadow-2xl relative group">
                         <div 
                            className="relative transition-transform duration-500 ease-in-out"
                            style={{ transform: `rotate(${settings.rotation}deg)` }}
                        >
                            {isPurifying ? (
                                <div className="w-[320px] h-[320px] rounded-full overflow-hidden shadow-2xl bg-black relative z-10 flex items-center justify-center">
                                    {isScreenSleep ? (
                                        <div className="w-full h-full bg-black" />
                                    ) : (
                                        <PurificationScreen />
                                    )}
                                </div>
                            ) : showSettings ? (
                                <div className="w-[320px] h-[320px] rounded-full overflow-hidden shadow-2xl bg-black relative z-10">
                                    {isScreenSleep ? (
                                        <div className="w-full h-full bg-black" />
                                    ) : (
                                        <SettingsScreen 
                                            settings={settings}
                                            updateSettings={updateSettings}
                                            currentView={settingsView}
                                            onViewChange={setSettingsView}
                                            onStartPurification={handleStartPurification}
                                            theme="light" // Force Light
                                            encoderRotateEvent={rotateEvent}
                                            encoderPressCount={pressCount}
                                        />
                                    )}
                                </div>
                            ) : (
                                <div className="w-[320px] h-[320px] rounded-full overflow-hidden shadow-2xl relative z-10">
                                     {isScreenSleep ? (
                                        <div className="w-full h-full bg-black" />
                                     ) : (
                                        <SegmentedRingDisplay 
                                            speed={speed} 
                                            theme="light" 
                                            filterStatus={filterStatus} 
                                            isLocked={isLocked} 
                                            isBooting={isBooting}
                                            accentColor={settings.accentColor}
                                        />
                                     )}
                                </div>
                            )}
                        </div>
                        {/* Reflection/Glare effect */}
                        <div className="absolute inset-0 rounded-[3rem] bg-gradient-to-tr from-black/0 via-white/20 to-transparent pointer-events-none opacity-30" />
                    </div>
                </div>

            </div>
            
        </div>
      </main>
    </div>
  );
}
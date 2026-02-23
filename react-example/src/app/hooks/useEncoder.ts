import { useState, useEffect, useCallback, useRef } from 'react';

export function useEncoder() {
  const [rotation, setRotation] = useState(0);
  const [isPressed, setIsPressed] = useState(false);
  
  // One-shot events
  const [rotateEvent, setRotateEvent] = useState<1 | -1 | 0>(0);
  const [pressCount, setPressCount] = useState(0);
  const [longPressCount, setLongPressCount] = useState(0);

  const pressStartTime = useRef<number | null>(null);
  const longPressTimer = useRef<NodeJS.Timeout | null>(null);
  const longPressTriggered = useRef(false);

  // 2 Seconds for long press as requested
  const LONG_PRESS_DURATION = 2000; 

  const handleRotate = useCallback((dir: 1 | -1) => {
    setRotation(r => r + dir);
    setRotateEvent(dir);
    // Clear the event immediately after render so it doesn't persist
    setTimeout(() => setRotateEvent(0), 10); 
  }, []);

  const handlePressDown = useCallback(() => {
    if (pressStartTime.current !== null) return; // already pressed
    setIsPressed(true);
    pressStartTime.current = Date.now();
    longPressTriggered.current = false;

    longPressTimer.current = setTimeout(() => {
        longPressTriggered.current = true;
        setLongPressCount(c => c + 1);
    }, LONG_PRESS_DURATION);
  }, []);

  const handlePressUp = useCallback(() => {
    if (pressStartTime.current === null) return;
    
    if (longPressTimer.current) {
        clearTimeout(longPressTimer.current);
        longPressTimer.current = null;
    }

    if (!longPressTriggered.current) {
        // Short press
        setPressCount(c => c + 1);
    }

    setIsPressed(false);
    pressStartTime.current = null;
  }, []);

  // Keyboard listeners
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
        // For rotation, we allow repeats (holding arrow key spins it)
        if (e.key === 'ArrowRight' || e.key === 'ArrowUp') {
            handleRotate(1);
        } else if (e.key === 'ArrowLeft' || e.key === 'ArrowDown') {
            handleRotate(-1);
        } else if (e.key === 'Enter' || e.key === ' ') {
            if (!e.repeat) handlePressDown();
        }
    };

    const handleKeyUp = (e: KeyboardEvent) => {
        if (e.key === 'Enter' || e.key === ' ') {
            handlePressUp();
        }
    };

    window.addEventListener('keydown', handleKeyDown);
    window.addEventListener('keyup', handleKeyUp);
    return () => {
        window.removeEventListener('keydown', handleKeyDown);
        window.removeEventListener('keyup', handleKeyUp);
    };
  }, [handleRotate, handlePressDown, handlePressUp]);

  return {
    rotation, // Total accumulated count
    rotateEvent, // +1 or -1 (transient)
    isPressed, // Current state
    pressCount,
    longPressCount,
    manualRotate: handleRotate,
    manualPressDown: handlePressDown,
    manualPressUp: handlePressUp
  };
}

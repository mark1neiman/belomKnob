import React, { useState, useRef, useEffect } from "react";
import {
  ChevronLeft,
  Globe,
  Sun,
  RotateCw,
  Lock,
  Filter,
  Info,
  ChevronRight,
  Check,
  RefreshCw,
  Moon,
  Palette,
  Gauge,
  Zap,
  AlertTriangle,
  Wind,
  Clock,
  Calendar,
} from "lucide-react";

// --- Types ---

export type Theme = "dark" | "light";
export type Language = "en" | "ru" | "de" | "fr";
export type Rotation = 0 | 180;

export interface SettingsState {
  language: Language;
  brightness: number; // 0-100
  theme: Theme;
  rotation: Rotation;
  lockMode: boolean;
  filterLifetime: number; // Hours, default 100
  filterUsed: number; // Hours
  sensitivity: number; // 15-100 turns for 100%
  accentColor: string; // Hex code
  allowOverdrive: boolean; // Allow speed up to 110% when filter is critical
}

interface SettingsScreenProps {
  settings: SettingsState;
  updateSettings: (updates: Partial<SettingsState>) => void;
  currentView: string;
  onViewChange: (view: string) => void;
  onStartPurification: () => void;
  theme?: Theme; // Optional override for previewing specific themes
  encoderRotateEvent?: 1 | -1 | 0; // -1, 0, 1
  encoderPressCount?: number;
}

// --- Constants ---

const LANGUAGES: { id: Language; label: string }[] = [
  { id: "en", label: "English" },
  { id: "ru", label: "Русский" },
  { id: "de", label: "Deutsch" },
  { id: "fr", label: "Français" },
];

const THEMES: { id: Theme; label: string }[] = [
  { id: "dark", label: "Dark" },
  { id: "light", label: "Light" },
];

const ACCENT_COLORS = [
  { label: "Red", value: "#E10600" },
  { label: "Orange", value: "#F97316" },
  { label: "Amber", value: "#EAB308" },
  { label: "Green", value: "#10B981" },
  { label: "Cyan", value: "#06B6D4" },
  { label: "Blue", value: "#3B82F6" },
  { label: "Purple", value: "#8B5CF6" },
  { label: "Pink", value: "#EC4899" },
];

// --- Wheel List Component ---

interface WheelListProps {
  children: React.ReactNode;
  className?: string;
  itemHeight?: number;
  onScroll?: (scrollTop: number) => void;
  selectedIndex?: number;
}

const WheelList = ({
  children,
  className = "",
  itemHeight = 80,
  selectedIndex,
}: WheelListProps) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const [scrollTop, setScrollTop] = useState(0);

  const handleScroll = () => {
    if (containerRef.current) {
      setScrollTop(containerRef.current.scrollTop);
    }
  };

  // Scroll to selected index when it changes
  useEffect(() => {
    if (selectedIndex !== undefined && containerRef.current) {
      containerRef.current.scrollTo({
        top: selectedIndex * itemHeight,
        behavior: "smooth",
      });
    }
  }, [selectedIndex, itemHeight]);

  // Convert children to array to clone them with props
  const items = React.Children.toArray(children);

  // Padding to allow the first and last items to reach the center
  const paddingY = 160 - itemHeight / 2;

  return (
    <div
      ref={containerRef}
      onScroll={handleScroll}
      className={`flex-1 w-full overflow-y-auto no-scrollbar relative perspective-500 snap-y snap-mandatory ${className}`}
    >
      <div style={{ height: paddingY, flexShrink: 0 }} />

      {items.map((_, index) => (
        <div
          key={`snap-${index}`}
          style={{
            height: itemHeight,
            scrollSnapAlign: "center",
            width: "100%",
          }}
        />
      ))}

      <div style={{ height: paddingY, flexShrink: 0 }} />

      {items.map((child, index) => {
        const scrollTopVal = scrollTop;
        // Calculate distance from center
        const distance = Math.abs(
          scrollTopVal - index * itemHeight,
        );

        // Normalize distance based on some range (e.g., 160px)
        const maxDist = 160;
        const normalizedDist =
          Math.min(distance, maxDist) / maxDist; // 0 (center) to 1 (edge)

        // Calculate visual properties
        const scale = 1 - normalizedDist * 0.35;
        const opacity = 1 - normalizedDist * 0.7;

        // RotateX for 3D tumbling effect
        const signedDist = index * itemHeight - scrollTopVal;
        const rotateX = -signedDist * 0.15; // deg

        return (
          <div
            key={index}
            className="flex items-center justify-center w-full absolute left-0 right-0 origin-center transition-transform duration-75 ease-out will-change-transform pointer-events-none"
            style={{
              height: itemHeight,
              top: paddingY + index * itemHeight, // Absolute positioning matching the flow content
              transform: `scale(${scale}) rotateX(${rotateX}deg) translateY(${signedDist * 0.1}px)`,
              opacity: Math.max(opacity, 0),
              zIndex: Math.round(100 - normalizedDist * 10),
              filter: `blur(${normalizedDist * 2}px)`,
            }}
          >
            {/* Re-enable pointer events on the child content itself so buttons work */}
            <div
              className={`pointer-events-auto ${normalizedDist > 0.6 ? "pointer-events-none" : ""}`}
            >
              {child}
            </div>
          </div>
        );
      })}
    </div>
  );
};

// --- Main Component ---

export function SettingsScreen({
  settings,
  updateSettings,
  currentView,
  onViewChange,
  onStartPurification,
  theme,
  encoderRotateEvent,
  encoderPressCount,
}: SettingsScreenProps) {
  // Use the prop theme if provided, otherwise fallback to settings.theme
  const activeTheme = theme || settings.theme;
  const isDark = activeTheme === "dark";
  const brandColor = settings.accentColor || "#E10600";

  // --- Root Menu ---
  const MENU_ITEMS = [
    {
      id: "sensitivity",
      icon: Gauge,
      label: "Sensitivity",
      value: `${settings.sensitivity} clicks`,
    },
    {
      id: "accentColor",
      icon: Palette,
      label: "Accent Color",
      value:
        ACCENT_COLORS.find(
          (c) => c.value === settings.accentColor,
        )?.label || "Custom",
    },
    {
      id: "language",
      icon: Globe,
      label: "Language",
      value: LANGUAGES.find((l) => l.id === settings.language)
        ?.label,
    },
    {
      id: "brightness",
      icon: Sun,
      label: "Brightness",
      value: `${settings.brightness}%`,
    },
    {
      id: "theme",
      icon: isDark ? Sun : Sun,
      label: "Theme",
      value: settings.theme === "dark" ? "Dark" : "Light",
    },
    {
      id: "rotation",
      icon: RotateCw,
      label: "Rotation",
      value: `${settings.rotation}°`,
    },
    {
      id: "lock",
      icon: Lock,
      label: "Lock Mode",
      value: settings.lockMode ? "On" : "Off",
    },
    {
      id: "overdriveWarning",
      icon: Zap,
      label: "Overdrive Mode",
      value: settings.allowOverdrive ? "Enabled" : "Disabled",
    },
    {
      id: "filter",
      icon: Filter,
      label: "Filter",
      value: `${settings.filterLifetime}h / ${Math.round(((settings.filterLifetime - settings.filterUsed) / settings.filterLifetime) * 100)}%`,
    },
    {
      id: "purificationConfirm",
      icon: Wind,
      label: "Air Purification",
      value: "Clean Air Mode",
    },
    { id: "about", icon: Info, label: "About", value: null },
  ];

  // --- Filter Submenu ---
  const FILTER_MENU_ITEMS = [
    {
      id: "filterLifetime",
      label: "Set Lifetime",
      value: `${settings.filterLifetime}h`,
    },
    {
      id: "filterReset",
      label: "Reset Usage",
      value: `${settings.filterUsed}h used`,
    },
    {
      id: "back",
      label: "Back",
      value: "Return to Menu",
    },
  ];

  // Helper to render the common header
  const Header = ({ title }: { title: string }) => (
    <div
      className={`absolute top-6 left-0 right-0 h-12 flex items-center justify-center z-50 pointer-events-none transition-opacity duration-300`}
    >
      <h2
        className={`text-sm font-bold uppercase tracking-widest opacity-60 ${isDark ? "text-white" : "text-zinc-900"}`}
      >
        {title}
      </h2>
    </div>
  );

  // --- Encoder Logic ---

  const [selectedIndex, setSelectedIndex] = useState(0);

  // Reset index when view changes
  useEffect(() => {
    setSelectedIndex(0);
  }, [currentView]);

  // Handle Rotation
  useEffect(() => {
    if (encoderRotateEvent) {
      if (currentView === "root") {
        setSelectedIndex((prev) =>
          Math.max(
            0,
            Math.min(
              MENU_ITEMS.length - 1,
              prev + encoderRotateEvent,
            ),
          ),
        );
      } else if (currentView === "brightness") {
        const step = 10;
        const newBr = Math.max(
          10,
          Math.min(
            100,
            settings.brightness + encoderRotateEvent * step,
          ),
        );
        if (newBr !== settings.brightness)
          updateSettings({ brightness: newBr });
      } else if (currentView === "sensitivity") {
        // Range 15-100, step 5
        const step = 1;
        const newSense = Math.max(
          15,
          Math.min(
            100,
            settings.sensitivity + encoderRotateEvent * step,
          ),
        );
        if (newSense !== settings.sensitivity)
          updateSettings({ sensitivity: newSense });
      } else if (currentView === "filterLifetime") {
        // Range 50-100, step 5
        const step = 5;
        const newLifetime = Math.max(
          50,
          Math.min(
            100,
            settings.filterLifetime + encoderRotateEvent * step,
          ),
        );
        if (newLifetime !== settings.filterLifetime)
          updateSettings({ filterLifetime: newLifetime });
      } else if (currentView === "accentColor") {
        setSelectedIndex((prev) =>
          Math.max(
            0,
            Math.min(
              ACCENT_COLORS.length - 1,
              prev + encoderRotateEvent,
            ),
          ),
        );
      } else if (currentView === "language") {
        setSelectedIndex((prev) =>
          Math.max(
            0,
            Math.min(
              LANGUAGES.length - 1,
              prev + encoderRotateEvent,
            ),
          ),
        );
      } else if (currentView === "theme") {
        setSelectedIndex((prev) =>
          Math.max(
            0,
            Math.min(
              THEMES.length - 1,
              prev + encoderRotateEvent,
            ),
          ),
        );
      } else if (currentView === "rotation") {
        setSelectedIndex((prev) =>
          Math.max(0, Math.min(1, prev + encoderRotateEvent)),
        );
      } else if (currentView === "lock") {
        setSelectedIndex((prev) =>
          Math.max(0, Math.min(1, prev + encoderRotateEvent)),
        );
      } else if (currentView === "overdrive") {
        setSelectedIndex((prev) =>
          Math.max(0, Math.min(1, prev + encoderRotateEvent)),
        );
      } else if (currentView === "filter") {
        // Filter submenu navigation
        setSelectedIndex((prev) =>
          Math.max(
            0,
            Math.min(
              FILTER_MENU_ITEMS.length - 1,
              prev + encoderRotateEvent,
            ),
          ),
        );
      } else if (currentView === "filterReset") {
        // 0 = Back, 1 = Reset
        setSelectedIndex((prev) =>
            Math.max(0, Math.min(1, prev + encoderRotateEvent))
        );
      } else if (currentView === "overdriveWarning") {
        // 0 = Back, 1 = Continue
        setSelectedIndex((prev) =>
            Math.max(0, Math.min(1, prev + encoderRotateEvent))
        );
      } else if (currentView === "purificationConfirm") {
        // 0 = Back, 1 = Start
        setSelectedIndex((prev) =>
            Math.max(0, Math.min(1, prev + encoderRotateEvent))
        );
      }
    }
  }, [encoderRotateEvent, currentView]);

  // Handle Press
  const lastProcessedPressCount = useRef(
    encoderPressCount || 0,
  );
  useEffect(() => {
    if (
      encoderPressCount &&
      encoderPressCount > lastProcessedPressCount.current
    ) {
      lastProcessedPressCount.current = encoderPressCount;

      if (currentView === "root") {
        const item = MENU_ITEMS[selectedIndex];
        if (item) onViewChange(item.id);
      } else if (currentView === "brightness") {
        onViewChange("root");
      } else if (currentView === "sensitivity") {
        onViewChange("root");
      } else if (currentView === "filterLifetime") {
        onViewChange("filter");
      } else if (currentView === "accentColor") {
        updateSettings({
          accentColor: ACCENT_COLORS[selectedIndex].value,
        });
        onViewChange("root");
      } else if (currentView === "language") {
        updateSettings({
          language: LANGUAGES[selectedIndex].id,
        });
        onViewChange("root");
      } else if (currentView === "theme") {
        updateSettings({ theme: THEMES[selectedIndex].id });
        onViewChange("root");
      } else if (currentView === "rotation") {
        const rots = [0, 180] as const;
        updateSettings({ rotation: rots[selectedIndex] });
        onViewChange("root");
      } else if (currentView === "lock") {
        const modes = [true, false];
        updateSettings({ lockMode: modes[selectedIndex] });
        onViewChange("root");
      } else if (currentView === "overdrive") {
        const modes = [true, false];
        updateSettings({ allowOverdrive: modes[selectedIndex] });
        onViewChange("root");
      } else if (currentView === "filter") {
        // Navigate to filter submenu items
        const filterItem = FILTER_MENU_ITEMS[selectedIndex];
        if (filterItem) {
            if (filterItem.id === "back") {
                onViewChange("root");
            } else {
                onViewChange(filterItem.id);
            }
        }
      } else if (currentView === "filterReset") {
        if (selectedIndex === 0) {
          // Back
          onViewChange("filter");
        } else {
          // Reset filter usage
          updateSettings({ filterUsed: 0 });
          onViewChange("filter");
        }
      } else if (currentView === "overdriveWarning") {
        if (selectedIndex === 0) {
          // Back
          onViewChange("root");
        } else {
          // Continue
          onViewChange("overdrive");
        }
      } else if (currentView === "purificationConfirm") {
        if (selectedIndex === 0) {
          // Back
          onViewChange("root");
        } else {
          // Start Purification
          onStartPurification();
        }
      } else if (currentView === "about") {
        onViewChange("root");
      }
    }
  }, [
    encoderPressCount,
    currentView,
    selectedIndex,
    MENU_ITEMS,
    FILTER_MENU_ITEMS,
    onViewChange,
    updateSettings,
  ]);

  // --- Sub-Views ---

  const renderSensitivity = () => (
    <>
      <Header title="Sensitivity" />
      <div className="flex-1 flex flex-col justify-center items-center w-full px-10 relative z-10">
        <div
          className={`text-6xl font-bold mb-6 tabular-nums tracking-tighter ${isDark ? "text-white" : "text-zinc-900"}`}
        >
          {settings.sensitivity}
        </div>
        <div
          className={`text-xs mb-8 uppercase tracking-widest ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
        >
          Clicks per 100%
        </div>

        <div className="w-full relative h-12 flex items-center px-4">
          <input
            type="range"
            min="15"
            max="100"
            step="1"
            value={settings.sensitivity}
            onChange={(e) =>
              updateSettings({
                sensitivity: parseInt(e.target.value),
              })
            }
            className="w-full appearance-none h-4 bg-zinc-700/50 rounded-full outline-none slider-thumb-color z-20 relative backdrop-blur-sm"
            style={{
              background: `linear-gradient(to right, ${brandColor} 0%, ${brandColor} ${((settings.sensitivity - 15) / (100 - 15)) * 100}%, ${isDark ? "rgba(63, 63, 70, 0.5)" : "rgba(228, 228, 231, 0.5)"} ${((settings.sensitivity - 15) / (100 - 15)) * 100}%, ${isDark ? "rgba(63, 63, 70, 0.5)" : "rgba(228, 228, 231, 0.5)"} 100%)`,
            }}
          />
        </div>
      </div>
    </>
  );

  const renderAccentColor = () => (
    <>
      <Header title="Accent Color" />
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {ACCENT_COLORS.map((color, index) => {
          const isSelected = settings.accentColor === color.value;
          const isFocused = index === selectedIndex;

          return (
            <button
              key={color.value}
              onClick={() => updateSettings({ accentColor: color.value })}
              className={`w-[260px] h-[70px] flex items-center justify-between px-6 rounded-2xl transition-all border-2 ${
                isSelected
                  ? isDark
                    ? "bg-zinc-900"
                    : "bg-zinc-100 shadow-sm"
                  : "bg-transparent"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/50"
                    : "border-zinc-400 bg-zinc-50"
                  : isSelected
                    ? isDark
                      ? "border-zinc-800"
                      : "border-zinc-200"
                    : "border-transparent opacity-50"
              }`}
            >
              <div className="flex items-center gap-4">
                <div
                  className="w-8 h-8 rounded-full border border-white/20 shadow-lg"
                  style={{ backgroundColor: color.value }}
                />
                <span
                  className={`text-xl font-medium ${isDark ? "text-zinc-100" : "text-zinc-900"}`}
                >
                  {color.label}
                </span>
              </div>
              {settings.accentColor === color.value && (
                <Check size={24} color={brandColor} />
              )}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderLanguage = () => (
    <>
      <Header title="Language" />
      <WheelList itemHeight={70} selectedIndex={selectedIndex}>
        {LANGUAGES.map((lang, index) => {
          const isSelected = settings.language === lang.id;
          const isFocused = index === selectedIndex;
          return (
            <button
              key={lang.id}
              onClick={() => updateSettings({ language: lang.id })}
              className={`w-[260px] h-[60px] flex items-center justify-between px-6 rounded-2xl transition-all border-2 ${
                isSelected
                  ? isDark
                    ? "bg-zinc-900"
                    : "bg-zinc-100 shadow-sm"
                  : "bg-transparent"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/50"
                    : "border-zinc-400 bg-zinc-50"
                  : isSelected
                    ? isDark
                      ? "border-zinc-800"
                      : "border-zinc-200"
                    : "border-transparent opacity-50"
              }`}
            >
              <span
                className={`text-lg font-medium ${isDark ? "text-zinc-100" : "text-zinc-900"}`}
              >
                {lang.label}
              </span>
              {isSelected && <Check size={20} color={brandColor} />}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderBrightness = () => (
    <>
      <Header title="Brightness" />
      <div className="flex-1 flex flex-col justify-center items-center w-full px-10 relative z-10">
        <div
          className={`text-6xl font-bold mb-6 tabular-nums tracking-tighter ${isDark ? "text-white" : "text-zinc-900"}`}
        >
          {settings.brightness}
          <span className="text-2xl text-zinc-500 align-top ml-1">
            %
          </span>
        </div>

        <div className="w-full relative h-12 flex items-center px-4">
          <input
            type="range"
            min="10"
            max="100"
            step="10"
            value={settings.brightness}
            onChange={(e) =>
              updateSettings({
                brightness: parseInt(e.target.value),
              })
            }
            className="w-full appearance-none h-4 bg-zinc-700/50 rounded-full outline-none slider-thumb-color z-20 relative backdrop-blur-sm"
            style={{
              background: `linear-gradient(to right, ${brandColor} 0%, ${brandColor} ${settings.brightness}%, ${isDark ? "rgba(63, 63, 70, 0.5)" : "rgba(228, 228, 231, 0.5)"} ${settings.brightness}%, ${isDark ? "rgba(63, 63, 70, 0.5)" : "rgba(228, 228, 231, 0.5)"} 100%)`,
            }}
          />
        </div>
      </div>
    </>
  );

  const renderTheme = () => (
    <>
      <Header title="Theme" />
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {THEMES.map((t, index) => {
          const isSelected = settings.theme === t.id;
          const isFocused = index === selectedIndex;
          return (
            <button
              key={t.id}
              onClick={() => updateSettings({ theme: t.id })}
              className={`w-[260px] h-[70px] flex items-center justify-between px-6 rounded-2xl transition-all border-2 ${
                isSelected
                  ? isDark
                    ? "bg-zinc-900"
                    : "bg-zinc-100 shadow-sm"
                  : "bg-transparent"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/50"
                    : "border-zinc-400 bg-zinc-50"
                  : isSelected
                    ? isDark
                      ? "border-zinc-800"
                      : "border-zinc-200"
                    : "border-transparent opacity-50"
              }`}
            >
              <div className="flex items-center gap-4">
                {t.id === "light" ? (
                  <Sun size={24} />
                ) : (
                  <Moon size={24} />
                )}
                <span
                  className={`text-xl font-medium ${isDark ? "text-zinc-100" : "text-zinc-900"}`}
                >
                  {t.label}
                </span>
              </div>
              {isSelected && <Check size={24} color={brandColor} />}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderRotation = () => (
    <>
      <Header title="Rotation" />
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {[0, 180].map((deg, index) => {
          const isSelected = settings.rotation === deg;
          const isFocused = index === selectedIndex;
          return (
            <button
              key={deg}
              onClick={() =>
                updateSettings({ rotation: deg as Rotation })
              }
              className={`w-[260px] h-[70px] flex items-center justify-between px-6 rounded-2xl transition-all border-2 ${
                isSelected
                  ? isDark
                    ? "bg-zinc-900"
                    : "bg-zinc-100 shadow-sm"
                  : "bg-transparent"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/50"
                    : "border-zinc-400 bg-zinc-50"
                  : isSelected
                    ? isDark
                      ? "border-zinc-800"
                      : "border-zinc-200"
                    : "border-transparent opacity-50"
              }`}
            >
              <span
                className={`text-xl font-medium ${isDark ? "text-zinc-100" : "text-zinc-900"}`}
              >
                {deg}°
              </span>
              {isSelected && <Check size={24} color={brandColor} />}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderLockMode = () => (
    <>
      <Header title="Lock Mode" />
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {[true, false].map((enabled, index) => {
          const isSelected = settings.lockMode === enabled;
          const isFocused = index === selectedIndex;
          return (
            <button
              key={enabled.toString()}
              onClick={() =>
                updateSettings({ lockMode: enabled })
              }
              className={`w-[260px] h-[70px] flex items-center justify-between px-6 rounded-2xl transition-all border-2 ${
                isSelected
                  ? isDark
                    ? "bg-zinc-900"
                    : "bg-zinc-100 shadow-sm"
                  : "bg-transparent"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/50"
                    : "border-zinc-400 bg-zinc-50"
                  : isSelected
                    ? isDark
                      ? "border-zinc-800"
                      : "border-zinc-200"
                    : "border-transparent opacity-50"
              }`}
            >
              <div className="flex flex-col items-start">
                <span
                  className={`text-xl font-medium ${isDark ? "text-zinc-100" : "text-zinc-900"}`}
                >
                  {enabled ? "Auto Lock" : "Always Unlocked"}
                </span>
                <span className="text-[10px] opacity-60">
                  {enabled
                    ? "Prevents accidental touches"
                    : "Quick access enabled"}
                </span>
              </div>
              {isSelected && <Check size={24} color={brandColor} />}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderFilter = () => (
    <>
      <Header title="Filter" />
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {FILTER_MENU_ITEMS.map((item, index) => {
          const isFocused = index === selectedIndex;
          return (
            <button
              key={item.id}
              onClick={() => {
                if (item.id === "back") {
                  onViewChange("root");
                } else {
                  onViewChange(item.id);
                }
              }}
              className={`w-[260px] h-[72px] flex items-center justify-between px-5 rounded-[20px] transition-all border-2 ${
                isDark
                  ? "bg-zinc-900/50 backdrop-blur-sm"
                  : "bg-white shadow-sm"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/80"
                    : "border-zinc-400 bg-zinc-50"
                  : isDark
                    ? "border-zinc-800"
                    : "border-zinc-100"
              }`}
            >
              {item.id === "back" ? (
                <div className="flex items-center justify-center w-full gap-2">
                  <ChevronLeft
                    size={18}
                    className={isDark ? "text-zinc-400" : "text-zinc-500"}
                  />
                  <span
                    className={`font-bold text-base ${isDark ? "text-zinc-100" : "text-zinc-800"}`}
                  >
                    {item.label}
                  </span>
                </div>
              ) : (
                <>
                  <div className="flex flex-col items-start">
                    <div
                      className={`font-bold text-base leading-tight ${isDark ? "text-zinc-100" : "text-zinc-800"}`}
                    >
                      {item.label}
                    </div>
                    <div
                      className={`text-[11px] font-medium leading-tight mt-0.5 ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
                    >
                      {item.value}
                    </div>
                  </div>
                  <ChevronRight
                    size={18}
                    className={`mr-1 opacity-50 ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
                  />
                </>
              )}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderFilterLifetime = () => (
    <>
      <Header title="Filter Lifetime" />
      <div className="flex-1 flex flex-col justify-center items-center w-full px-10 relative z-10">
        <div
          className={`text-6xl font-bold mb-6 tabular-nums tracking-tighter ${isDark ? "text-white" : "text-zinc-900"}`}
        >
          {settings.filterLifetime}
          <span className="text-2xl text-zinc-500 align-top ml-1">
            h
          </span>
        </div>
        <div
          className={`text-xs mb-8 uppercase tracking-widest ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
        >
          Total Hours
        </div>

        <div className="w-full relative h-12 flex items-center px-4">
          <input
            type="range"
            min="50"
            max="100"
            step="5"
            value={settings.filterLifetime}
            onChange={(e) =>
              updateSettings({
                filterLifetime: parseInt(e.target.value),
              })
            }
            className="w-full appearance-none h-4 bg-zinc-700/50 rounded-full outline-none slider-thumb-color z-20 relative backdrop-blur-sm"
            style={{
              background: `linear-gradient(to right, ${brandColor} 0%, ${brandColor} ${((settings.filterLifetime - 50) / (100 - 50)) * 100}%, ${isDark ? "rgba(63, 63, 70, 0.5)" : "rgba(228, 228, 231, 0.5)"} ${((settings.filterLifetime - 50) / (100 - 50)) * 100}%, ${isDark ? "rgba(63, 63, 70, 0.5)" : "rgba(228, 228, 231, 0.5)"} 100%)`,
            }}
          />
        </div>
      </div>
    </>
  );

  const renderFilterReset = () => (
    <>
      <div className="flex-1 flex flex-col items-center justify-center w-full px-10 pb-4">
        {/* Status Info */}
        <div
          className={`w-40 h-40 rounded-full flex flex-col items-center justify-center mb-8 border-4 relative ${isDark ? "bg-zinc-900 border-zinc-800" : "bg-zinc-100 border-zinc-200"}`}
        >
          <div
            className="absolute inset-0 rounded-full border-4 border-l-current border-t-transparent border-r-transparent border-b-transparent rotate-45"
            style={{ color: brandColor }}
          />

          <div
            className={`text-[10px] uppercase tracking-widest mb-1 ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
          >
            Used
          </div>
          <div
            className="text-5xl font-mono font-bold tracking-tight"
            style={{ color: brandColor }}
          >
            {settings.filterUsed}
            <span className="text-xl text-zinc-500 align-top ml-1">
              h
            </span>
          </div>
          <div
            className={`text-xs mt-1 ${isDark ? "text-zinc-400" : "text-zinc-500"}`}
          >
            of {settings.filterLifetime}h
          </div>
        </div>

        {/* Actions */}
        <div className="flex items-center gap-4">
          {/* BACK Button (0) */}
          <button
            onClick={() => onViewChange("filter")}
            className={`w-14 h-14 flex items-center justify-center rounded-full transition-all border-2 ${
              selectedIndex === 0
                ? isDark
                  ? "bg-zinc-100 text-zinc-900 border-zinc-100"
                  : "bg-zinc-900 text-zinc-100 border-zinc-900"
                : isDark
                  ? "bg-transparent text-zinc-500 border-zinc-800"
                  : "bg-transparent text-zinc-400 border-zinc-200"
            }`}
          >
            <ChevronLeft size={24} />
          </button>

          {/* RESET Button (1) */}
          <button
            onClick={() => {
              updateSettings({ filterUsed: 0 });
              onViewChange("filter");
            }}
            className={`flex items-center gap-2 px-6 h-14 rounded-full text-xs font-bold uppercase tracking-widest transition-all border-2 ${
              selectedIndex === 1
                ? "bg-red-600 text-white border-red-600 shadow-lg shadow-red-900/40"
                : isDark
                  ? "bg-transparent text-zinc-500 border-zinc-800"
                  : "bg-transparent text-zinc-400 border-zinc-200"
            }`}
          >
            <RefreshCw size={14} /> RESET
          </button>
        </div>
      </div>
    </>
  );

  const renderAbout = () => (
    <>
      <Header title="About" />
      <div
        className={`flex-1 flex flex-col items-center justify-center px-10 pb-10 text-center space-y-6 ${isDark ? "text-zinc-400" : "text-zinc-500"}`}
      >
        <div>
          <div
            className={`text-[10px] uppercase tracking-widest mb-2 ${isDark ? "text-zinc-600" : "text-zinc-400"}`}
          >
            Model
          </div>
          <div
            className={`text-3xl font-bold tracking-tight ${isDark ? "text-white" : "text-zinc-900"}`}
          >
            BELOM A2
          </div>
        </div>

        <div className="grid grid-cols-2 gap-8 w-full">
          <div className="flex flex-col items-center">
            <div
              className={`text-[9px] uppercase tracking-widest mb-1 ${isDark ? "text-zinc-600" : "text-zinc-400"}`}
            >
              Firmware
            </div>
            <div
              className={`font-mono text-lg ${isDark ? "text-zinc-300" : "text-zinc-700"}`}
            >
              2.4.1
            </div>
          </div>
          <div className="flex flex-col items-center">
            <div
              className={`text-[9px] uppercase tracking-widest mb-1 ${isDark ? "text-zinc-600" : "text-zinc-400"}`}
            >
              Interface
            </div>
            <div
              className={`font-mono text-lg ${isDark ? "text-zinc-300" : "text-zinc-700"}`}
            >
              1.0.0
            </div>
          </div>
        </div>
      </div>
    </>
  );

  const renderOverdriveWarning = () => (
    <>
      <div className="flex-1 flex flex-col items-center justify-center w-full px-6 py-4">
        {/* Warning Icon */}
        <div
          className={`w-14 h-14 rounded-full flex items-center justify-center mb-3 border-2 ${
            isDark ? "bg-amber-900/20 border-amber-800" : "bg-amber-100 border-amber-300"
          }`}
        >
          <AlertTriangle size={24} className="text-amber-500" />
        </div>

        {/* Title */}
        <h3
          className={`text-lg font-bold mb-2 text-center ${isDark ? "text-white" : "text-zinc-900"}`}
        >
          Overdrive Mode
        </h3>

        {/* Warning Text */}
        <div
          className={`text-[10px] leading-tight text-center space-y-1 mb-4 ${isDark ? "text-zinc-400" : "text-zinc-600"}`}
        >
          <p>
            Allows fan speed up to <span className="font-bold" style={{color: brandColor}}>110%</span> when filter is critical
          </p>
          <p className="font-bold">
            Active only for 20 hours,
          </p>
          <p>
            then automatically returns to 100%
          </p>
        </div>

        {/* Actions */}
        <div className="flex items-center gap-4">
          {/* BACK Button (0) */}
          <button
            onClick={() => onViewChange("root")}
            className={`w-14 h-14 flex items-center justify-center rounded-full transition-all border-2 ${
              selectedIndex === 0
                ? isDark
                  ? "bg-zinc-100 text-zinc-900 border-zinc-100"
                  : "bg-zinc-900 text-zinc-100 border-zinc-900"
                : isDark
                  ? "bg-transparent text-zinc-500 border-zinc-800"
                  : "bg-transparent text-zinc-400 border-zinc-200"
            }`}
          >
            <ChevronLeft size={24} />
          </button>

          {/* Continue Button (1) */}
          <button
            onClick={() => onViewChange("overdrive")}
            className={`px-6 h-14 rounded-full text-xs font-bold uppercase tracking-widest transition-all border-2 ${
              selectedIndex === 1
                ? isDark
                  ? "bg-zinc-100 text-zinc-900 border-zinc-100"
                  : "bg-zinc-900 text-zinc-100 border-zinc-900"
                : isDark
                  ? "bg-zinc-800 text-zinc-500 border-zinc-800"
                  : "bg-zinc-200 text-zinc-400 border-zinc-200"
            }`}
          >
            Continue
          </button>
        </div>
      </div>
    </>
  );

  const renderPurificationConfirm = () => (
    <>
      <div className="flex-1 flex flex-col items-center justify-center w-full px-6 py-4">
        {/* Icon */}
        <div
          className={`w-14 h-14 rounded-full flex items-center justify-center mb-3 border-2 ${
            isDark ? "bg-emerald-900/20 border-emerald-800" : "bg-emerald-100 border-emerald-300"
          }`}
        >
          <Wind size={24} className="text-emerald-500" />
        </div>

        {/* Title */}
        <h3
          className={`text-lg font-bold mb-2 text-center ${isDark ? "text-white" : "text-zinc-900"}`}
        >
          Air Purification
        </h3>

        {/* Warning Text */}
        <div
          className={`text-[10px] leading-tight text-center space-y-1 mb-6 ${isDark ? "text-zinc-400" : "text-zinc-600"}`}
        >
          <p className="font-bold">
            Please ensure an appropriate filter
          </p>
          <p>
            is installed for air cleaning.
          </p>
          <p className="mt-2 opacity-70">
            Fan will run at 10% speed.
          </p>
        </div>

        {/* Actions */}
        <div className="flex items-center gap-4">
          {/* BACK Button (0) */}
          <button
            onClick={() => onViewChange("root")}
            className={`w-14 h-14 flex items-center justify-center rounded-full transition-all border-2 ${
              selectedIndex === 0
                ? isDark
                  ? "bg-zinc-100 text-zinc-900 border-zinc-100"
                  : "bg-zinc-900 text-zinc-100 border-zinc-900"
                : isDark
                  ? "bg-transparent text-zinc-500 border-zinc-800"
                  : "bg-transparent text-zinc-400 border-zinc-200"
            }`}
          >
            <ChevronLeft size={24} />
          </button>

          {/* Start Button (1) */}
          <button
            onClick={onStartPurification}
            className={`px-6 h-14 rounded-full text-xs font-bold uppercase tracking-widest transition-all border-2 ${
              selectedIndex === 1
                ? isDark
                  ? "bg-emerald-500 text-white border-emerald-500 shadow-lg shadow-emerald-900/40"
                  : "bg-emerald-500 text-white border-emerald-500 shadow-lg"
                : isDark
                  ? "bg-zinc-800 text-zinc-500 border-zinc-800"
                  : "bg-zinc-200 text-zinc-400 border-zinc-200"
            }`}
          >
            START
          </button>
        </div>
      </div>
    </>
  );

  const renderOverdrive = () => (
    <>
      <Header title="Overdrive Mode" />
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {[true, false].map((enabled, index) => {
          const isSelected = settings.allowOverdrive === enabled;
          const isFocused = index === selectedIndex;
          return (
            <button
              key={enabled.toString()}
              onClick={() =>
                updateSettings({ allowOverdrive: enabled })
              }
              className={`w-[260px] h-[70px] flex items-center justify-between px-6 rounded-2xl transition-all border-2 ${
                isSelected
                  ? isDark
                    ? "bg-zinc-900"
                    : "bg-zinc-100 shadow-sm"
                  : "bg-transparent"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/50"
                    : "border-zinc-400 bg-zinc-50"
                  : isSelected
                    ? isDark
                      ? "border-zinc-800"
                      : "border-zinc-200"
                    : "border-transparent opacity-50"
              }`}
            >
              <div className="flex flex-col items-start">
                <span
                  className={`text-xl font-medium ${isDark ? "text-zinc-100" : "text-zinc-900"}`}
                >
                  {enabled ? "Enabled" : "Disabled"}
                </span>
                <span className="text-[10px] opacity-60">
                  {enabled
                    ? "110% when filter critical"
                    : "Max 100% speed"}
                </span>
              </div>
              {isSelected && <Check size={24} color={brandColor} />}
            </button>
          );
        })}
      </WheelList>
    </>
  );

  const renderRoot = () => (
    <>
      <Header title="Settings" />

      {/* Scrollable Wheel List */}
      <WheelList itemHeight={80} selectedIndex={selectedIndex}>
        {MENU_ITEMS.map((item, index) => {
          const isFocused = index === selectedIndex;
          return (
            <button
              key={item.id}
              onClick={() => onViewChange(item.id)}
              className={`w-[260px] h-[72px] flex items-center justify-between px-5 rounded-[20px] group transition-all duration-300 border-2 ${
                isDark
                  ? "bg-zinc-900/50 backdrop-blur-sm"
                  : "bg-white shadow-sm"
              } ${
                isFocused
                  ? isDark
                    ? "border-zinc-400 bg-zinc-800/80"
                    : "border-zinc-400 bg-zinc-50"
                  : isDark
                    ? "border-zinc-800"
                    : "border-zinc-100"
              }`}
            >
              <div className="flex items-center gap-4">
                <div
                  className={`p-2.5 rounded-full ${isDark ? "bg-zinc-800" : "bg-zinc-100"}`}
                >
                  {/* Dynamic Color Icon */}
                  <item.icon size={20} style={{ color: brandColor }} />
                </div>
                <div className="text-left">
                  <div
                    className={`font-bold text-base leading-tight ${isDark ? "text-zinc-100" : "text-zinc-800"}`}
                  >
                    {item.label}
                  </div>
                  {item.value && (
                    <div
                      className={`text-[11px] font-medium leading-tight mt-0.5 ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
                    >
                      {item.value}
                    </div>
                  )}
                </div>
              </div>
              <ChevronRight
                size={18}
                className={`mr-1 opacity-50 ${isDark ? "text-zinc-500" : "text-zinc-400"}`}
              />
            </button>
          );
        })}
      </WheelList>
    </>
  );

  return (
    <div
      className={`w-[320px] h-[320px] rounded-full relative overflow-hidden flex flex-col shadow-2xl ${
        isDark ? "bg-zinc-950" : "bg-[#F2F2F2]"
      }`}
    >
      <style>{`
            .no-scrollbar::-webkit-scrollbar { display: none; }
            .no-scrollbar { -ms-overflow-style: none; scrollbar-width: none; }
            
            /* Custom Range Slider Thumb */
            input[type=range]::-webkit-slider-thumb {
                -webkit-appearance: none;
                height: 28px;
                width: 28px;
                border-radius: 50%;
                background: ${brandColor};
                cursor: pointer;
                border: 3px solid ${isDark ? "#09090b" : "#ffffff"};
                box-shadow: 0 4px 12px rgba(0,0,0,0.4);
            }

            /* Perspective container for 3D effects */
            .perspective-500 {
                perspective: 500px;
            }
        `}</style>

      {/* Subtle Ambient Glow */}
      <div
        className={`absolute inset-0 pointer-events-none opacity-20 bg-gradient-to-tr ${isDark ? "from-transparent via-transparent to-zinc-800" : "from-transparent via-transparent to-white"}`}
      />

      {currentView === "root" && renderRoot()}
      {currentView === "sensitivity" && renderSensitivity()}
      {currentView === "accentColor" && renderAccentColor()}
      {currentView === "language" && renderLanguage()}
      {currentView === "brightness" && renderBrightness()}
      {currentView === "theme" && renderTheme()}
      {currentView === "rotation" && renderRotation()}
      {currentView === "lock" && renderLockMode()}
      {currentView === "filter" && renderFilter()}
      {currentView === "filterLifetime" &&
        renderFilterLifetime()}
      {currentView === "filterReset" && renderFilterReset()}
      {currentView === "about" && renderAbout()}
      {currentView === "overdriveWarning" && renderOverdriveWarning()}
      {currentView === "overdrive" && renderOverdrive()}
      {currentView === "purificationConfirm" && renderPurificationConfirm()}
    </div>
  );
}
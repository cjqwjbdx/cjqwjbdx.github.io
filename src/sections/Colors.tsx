import { useState, useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';

gsap.registerPlugin(ScrollTrigger);

interface ColorOption {
  name: string;
  code: string;
  color: string;
  category: string;
  image: string;
}

const colors: ColorOption[] = [
  // 单色系
  { name: '金栗', code: 'JL', color: '#F5C400', category: '单色', image: '/images/product-jinli.png' },
  { name: '靛澜', code: 'DL', color: '#2491ff', category: '单色', image: '/images/product-dianlan.png' },
  { name: '柳烟', code: 'LY', color: '#5EC57C', category: '单色', image: '/images/product-liuyan.png' },
  { name: '榴火', code: 'LH', color: '#c62222', category: '单色', image: '/images/product-liuhuo.png' },
  { name: '菡萏', code: 'HD', color: '#FFB6C1', category: '单色', image: '/images/product-handan.png' },
  { name: '天青', code: 'TQ', color: '#50C5E0', category: '单色', image: '/images/product-tianqing.png' },
  { name: '紫宸', code: 'ZC', color: '#6E3E8A', category: '单色', image: '/images/product-zichen.png' },
  // 透明系
  { name: '绯绡', code: 'FX', color: 'linear-gradient(135deg, rgba(208,13,13,0.6) 0%, rgba(208,13,13,0.3) 100%)', category: '透明', image: '/images/product-feixiao.png' },
  { name: '冰髓', code: 'BS', color: 'linear-gradient(135deg, rgba(200,200,200,0.5) 0%, rgba(255,255,255,0.3) 100%)', category: '透明', image: '/images/product-bingsui.png' },
  // 经典色
  { name: '墨缁', code: '', color: '#1D1D1F', category: '经典', image: '/images/product-black.png' },
  { name: '素缣', code: '', color: '#FFFFFF', category: '经典', image: '/images/product-sujian.png' },
  { name: '烟霭', code: '', color: '#86868B', category: '经典', image: '/images/product-yanai.png' },
];

const Colors = () => {
  const [selectedColor, setSelectedColor] = useState<ColorOption>(colors[9]); // Default to 墨缁
  const [isTransitioning, setIsTransitioning] = useState(false);
  const sectionRef = useRef<HTMLElement>(null);
  const titleRef = useRef<HTMLDivElement>(null);
  const productRef = useRef<HTMLDivElement>(null);
  const wheelRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const section = sectionRef.current;
    const title = titleRef.current;
    const product = productRef.current;
    const wheel = wheelRef.current;

    if (!section || !title || !product || !wheel) return;

    const ctx = gsap.context(() => {
      // Title animation
      gsap.fromTo(
        title,
        { opacity: 0, y: 30 },
        {
          opacity: 1,
          y: 0,
          duration: 0.6,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: title,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Product image animation
      gsap.fromTo(
        product,
        { opacity: 0, scale: 0.9 },
        {
          opacity: 1,
          scale: 1,
          duration: 0.8,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: product,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
          delay: 0.2,
        }
      );

      // Color wheel animation
      gsap.fromTo(
        wheel,
        { opacity: 0, rotate: -180 },
        {
          opacity: 1,
          rotate: 0,
          duration: 1,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: wheel,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
          delay: 0.4,
        }
      );

      // Color dots animation
      const dots = wheel.querySelectorAll('.color-dot-wrapper');
      dots.forEach((dot, index) => {
        gsap.fromTo(
          dot,
          { opacity: 0, scale: 0 },
          {
            opacity: 1,
            scale: 1,
            duration: 0.3,
            ease: 'back.out(1.7)',
            scrollTrigger: {
              trigger: wheel,
              start: 'top 80%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.5 + index * 0.05,
          }
        );
      });
    }, section);

    return () => ctx.revert();
  }, []);

  const handleColorClick = (color: ColorOption) => {
    if (color.name === selectedColor.name || isTransitioning) return;
    
    setIsTransitioning(true);
    
    // Fade out current image
    const product = productRef.current;
    if (product) {
      gsap.to(product.querySelector('img'), {
        opacity: 0,
        scale: 0.95,
        duration: 0.2,
        ease: 'power2.in',
        onComplete: () => {
          setSelectedColor(color);
          // Fade in new image
          gsap.to(product.querySelector('img'), {
            opacity: 1,
            scale: 1,
            duration: 0.3,
            ease: 'power2.out',
            onComplete: () => setIsTransitioning(false),
          });
        },
      });
    } else {
      setSelectedColor(color);
      setIsTransitioning(false);
    }
  };

  // Calculate position for each color dot in a circle
  const getDotPosition = (index: number, total: number) => {
    const angle = (index / total) * 360 - 90; // Start from top
    const radius = 180; // Radius in pixels
    const x = Math.cos((angle * Math.PI) / 180) * radius;
    const y = Math.sin((angle * Math.PI) / 180) * radius;
    return { x, y };
  };

  return (
    <section
      id="colors"
      ref={sectionRef}
      className="relative py-24 md:py-32 overflow-hidden"
      style={{ backgroundColor: 'var(--apple-light-gray)' }}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        {/* Section Title */}
        <div ref={titleRef} className="text-center mb-16">
          <h2 className="heading-section text-[var(--apple-dark)] mb-4">
            12种颜色，随心选择
          </h2>
          <p className="body-large">3D打印外壳，个性定制</p>
        </div>

        {/* Product with Color Wheel */}
        <div className="relative flex flex-col items-center">
          {/* Product Image */}
          <div ref={productRef} className="relative z-10 mb-12">
            <div className="relative">
              <img
                src={selectedColor.image}
                alt={`WuJin Calendar - ${selectedColor.name}`}
                className="w-full max-w-md h-auto rounded-2xl shadow-2xl transition-all duration-300"
              />
              {/* Selected Color Indicator */}
              <div className="absolute -bottom-4 left-1/2 -translate-x-1/2 flex items-center gap-2 px-4 py-2 bg-white rounded-full shadow-lg">
                <div
                  className="w-4 h-4 rounded-full border border-gray-200"
                  style={{
                    background: selectedColor.color,
                  }}
                />
                <span className="text-sm font-medium text-[var(--apple-dark)]">
                  {selectedColor.name}
                  {selectedColor.code && ` (${selectedColor.code})`}
                </span>
              </div>
            </div>
          </div>

          {/* Color Wheel */}
          <div
            ref={wheelRef}
            className="relative w-[400px] h-[400px] md:w-[500px] md:h-[500px]"
          >
            {/* Center Label */}
            <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
              <div className="text-center">
                <p className="text-sm text-[var(--apple-gray)]">选择颜色</p>
              </div>
            </div>

            {/* Color Dots */}
            {colors.map((color, index) => {
              const { x, y } = getDotPosition(index, colors.length);
              const isSelected = selectedColor.name === color.name;

              return (
                <div
                  key={color.name}
                  className="color-dot-wrapper absolute"
                  style={{
                    left: `calc(50% + ${x}px)`,
                    top: `calc(50% + ${y}px)`,
                    transform: 'translate(-50%, -50%)',
                  }}
                >
                  <button
                    onClick={() => handleColorClick(color)}
                    className={`color-dot relative ${isSelected ? 'active' : ''}`}
                    style={{
                      background: color.color,
                      boxShadow:
                        color.name === '素缣'
                          ? 'inset 0 0 0 1px rgba(0,0,0,0.1)'
                          : 'none',
                    }}
                    title={`${color.name} ${color.code ? `(${color.code})` : ''}`}
                  >
                    {/* Pulse ring for selected */}
                    {isSelected && (
                      <span
                        className="absolute inset-0 rounded-full animate-ping"
                        style={{
                          background: color.color,
                          opacity: 0.3,
                        }}
                      />
                    )}
                  </button>

                  {/* Tooltip */}
                  <div className="absolute -bottom-8 left-1/2 -translate-x-1/2 whitespace-nowrap opacity-0 hover:opacity-100 transition-opacity duration-200 pointer-events-none">
                    <span className="text-xs text-[var(--apple-gray)] bg-white px-2 py-1 rounded shadow">
                      {color.name}
                    </span>
                  </div>
                </div>
              );
            })}

            {/* Connecting Ring */}
            <div
              className="absolute inset-0 rounded-full border border-dashed border-gray-300"
              style={{
                margin: '60px',
              }}
            />
          </div>

          {/* Color Categories */}
          <div className="mt-12 flex flex-wrap justify-center gap-6">
            {['单色', '透明', '经典'].map((category) => (
              <div key={category} className="flex items-center gap-2">
                <span className="text-sm text-[var(--apple-gray)]">{category}</span>
                <div className="flex gap-1">
                  {colors
                    .filter((c) => c.category === category)
                    .slice(0, 3)
                    .map((color) => (
                      <div
                        key={color.name}
                        className="w-4 h-4 rounded-full"
                        style={{
                          background: color.color,
                          boxShadow:
                            color.name === '素缣'
                              ? 'inset 0 0 0 1px rgba(0,0,0,0.1)'
                              : 'none',
                        }}
                      />
                    ))}
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  );
};

export default Colors;

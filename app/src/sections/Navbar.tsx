import { useState, useEffect } from 'react';
import { Menu, X, ShoppingBag, Home } from 'lucide-react';

const Navbar = () => {
  const [isScrolled, setIsScrolled] = useState(false);
  const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false);

  useEffect(() => {
    const handleScroll = () => {
      setIsScrolled(window.scrollY > 50);
    };

    window.addEventListener('scroll', handleScroll, { passive: true });
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  const navLinks = [
    { name: '首页', href: '#hero' },
    { name: '产品', href: '#products' },
    { name: '功能', href: '#features' },
    { name: '颜色', href: '#colors' },
    { name: '规格', href: '#specs' },
  ];

  const scrollToSection = (href: string) => {
    const element = document.querySelector(href);
    if (element) {
      element.scrollIntoView({ behavior: 'smooth' });
    }
    setIsMobileMenuOpen(false);
  };

  return (
    <nav
      className={`fixed top-0 left-0 right-0 z-50 transition-all duration-400 ${
        isScrolled
          ? 'glass border-b border-gray-200/50'
          : 'bg-transparent'
      }`}
      style={{
        transitionTimingFunction: 'var(--ease-apple)',
      }}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-11">
          {/* Logo */}
          <a
            href="#hero"
            onClick={(e) => {
              e.preventDefault();
              scrollToSection('#hero');
            }}
            className="flex items-center gap-2 transition-transform duration-300"
            style={{
              transform: isScrolled ? 'scale(0.9)' : 'scale(1)',
            }}
          >
            <img
              src="/images/logo.jpeg"
              alt="WuJin Studio"
              className="h-6 w-auto"
            />
            <span className="text-sm font-semibold text-[var(--apple-dark)]">
              WuJin
            </span>
          </a>

          {/* Desktop Navigation */}
          <div className="hidden md:flex items-center gap-8">
            <a
              href="/index.html"
              className="flex items-center gap-1 text-sm text-[var(--apple-dark)] hover:text-[var(--apple-blue)] transition-colors duration-200"
            >
              <Home className="w-4 h-4" />
              返回主页
            </a>
            {navLinks.map((link, index) => (
              <a
                key={link.name}
                href={link.href}
                onClick={(e) => {
                  e.preventDefault();
                  scrollToSection(link.href);
                }}
                className="relative text-sm text-[var(--apple-dark)] hover:text-[var(--apple-blue)] transition-colors duration-200 group"
                style={{
                  animationDelay: `${index * 100}ms`,
                }}
              >
                {link.name}
                <span className="absolute -bottom-1 left-1/2 w-0 h-0.5 bg-[var(--apple-blue)] transition-all duration-200 group-hover:w-full group-hover:left-0" />
              </a>
            ))}
          </div>

          {/* CTA Button */}
          <div className="hidden md:flex items-center">
            <a
              href="https://www.goofish.com/personal?spm=a21ybx.item.itemHeader.1.c03b3da6zaL1ag&userId=2219084601045"
              target="_blank"
              rel="noopener noreferrer"
              className="flex items-center gap-2 px-4 py-1.5 bg-[var(--apple-blue)] text-white text-sm font-medium rounded-full hover:bg-[var(--apple-blue-hover)] transition-all duration-300 hover:scale-105"
            >
              <ShoppingBag className="w-4 h-4" />
              购买
            </a>
          </div>

          {/* Mobile Menu Button */}
          <button
            className="md:hidden p-2"
            onClick={() => setIsMobileMenuOpen(!isMobileMenuOpen)}
          >
            {isMobileMenuOpen ? (
              <X className="w-5 h-5 text-[var(--apple-dark)]" />
            ) : (
              <Menu className="w-5 h-5 text-[var(--apple-dark)]" />
            )}
          </button>
        </div>
      </div>

      {/* Mobile Menu */}
      <div
        className={`md:hidden absolute top-11 left-0 right-0 glass border-b border-gray-200/50 transition-all duration-300 ${
          isMobileMenuOpen
            ? 'opacity-100 translate-y-0'
            : 'opacity-0 -translate-y-4 pointer-events-none'
        }`}
      >
        <div className="px-4 py-4 space-y-3">
          <a
            href="/index.html"
            className="flex items-center gap-1 text-base text-[var(--apple-dark)] hover:text-[var(--apple-blue)] transition-colors duration-200"
          >
            <Home className="w-4 h-4" />
            返回主页
          </a>
          {navLinks.map((link, index) => (
            <a
              key={link.name}
              href={link.href}
              onClick={(e) => {
                e.preventDefault();
                scrollToSection(link.href);
              }}
              className="block text-base text-[var(--apple-dark)] hover:text-[var(--apple-blue)] transition-colors duration-200"
              style={{
                animationDelay: `${index * 50}ms`,
              }}
            >
              {link.name}
            </a>
          ))}
          <a
            href="https://www.goofish.com/personal?spm=a21ybx.item.itemHeader.1.c03b3da6zaL1ag&userId=2219084601045"
            target="_blank"
            rel="noopener noreferrer"
            className="w-full flex items-center justify-center gap-2 px-4 py-2 bg-[var(--apple-blue)] text-white text-base font-medium rounded-full hover:bg-[var(--apple-blue-hover)] transition-all duration-300"
          >
            <ShoppingBag className="w-4 h-4" />
            立即购买
          </a>
        </div>
      </div>
    </nav>
  );
};

export default Navbar;

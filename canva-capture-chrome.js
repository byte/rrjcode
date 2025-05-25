// 🎯 WORKING DEVELOPER TOOLS HACK FOR CANVA
// ==========================================
// STEP 1: Load html2canvas library
// ==========================================

console.log('🚀 Loading html2canvas library...');
const script = document.createElement('script');
script.src = 'https://cdnjs.cloudflare.com/ajax/libs/html2canvas/1.4.1/html2canvas.min.js';
script.onload = () => {
  console.log('✅ html2canvas loaded successfully!');
  console.log('🎯 Now run: startSlideCapture()');
};
document.head.appendChild(script);

// ===========================================
// STEP 2: Main capture function (WORKING VERSION)
// ===========================================

async function startSlideCapture() {
  console.log('🎯 STARTING CANVA SLIDE CAPTURE');
  console.log('===============================');
  
  const totalSlides = 77;
  const delay = (ms) => new Promise(resolve => setTimeout(resolve, ms));
  
  // Create download helper
  function downloadImage(dataUrl, filename) {
    const link = document.createElement('a');
    link.download = filename;
    link.href = dataUrl;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  }
  
  console.log(`📸 Starting capture of ${totalSlides} slides...`);
  console.log('💾 Downloads will appear in your Downloads folder');
  console.log('');
  
  for (let slideNum = 1; slideNum <= totalSlides; slideNum++) {
    try {
      console.log(`📍 SLIDE ${slideNum}/${totalSlides}`);
      
      // Navigate to slide
      console.log('🔗 Navigating...');
      window.location.hash = slideNum;
      
      // Wait for content to load
      console.log('⏳ Waiting for content...');
      await delay(6000); // 6 seconds wait
      
      // Force any lazy loading
      window.scrollTo(0, 100);
      window.scrollTo(0, 0);
      await delay(1000);
      
      // Find the slide content area (the working selectors)
      console.log('🎯 Finding slide content area...');
      
      const slideSelectors = [
        '[data-testid="design-canvas"]',
        '.canvas-container',
        '[class*="canvas"]',
        'main > div:first-child',
        '[role="main"] > div:first-child'
      ];
      
      let captureElement = null;
      for (const selector of slideSelectors) {
        const element = document.querySelector(selector);
        if (element && element.offsetWidth > 800) {
          captureElement = element;
          console.log(`✅ Found slide content: ${selector}`);
          break;
        }
      }
      
      // Fallback to main
      if (!captureElement) {
        captureElement = document.querySelector('main');
        console.log('⚠️ Using main element fallback');
      }
      
      console.log('📸 Capturing screenshot...');
      
      // Capture with html2canvas (WORKING SETTINGS)
      const canvas = await html2canvas(captureElement, {
        useCORS: true,
        allowTaint: true,
        backgroundColor: null,
        scale: 2,
        logging: false
      });
      
      // Convert to image and download
      const dataUrl = canvas.toDataURL('image/png', 1.0);
      const filename = `slide_${slideNum.toString().padStart(3, '0')}.png`;
      
      downloadImage(dataUrl, filename);
      
      console.log(`✅ Downloaded: ${filename}`);
      
      // Progress update
      const progress = ((slideNum / totalSlides) * 100).toFixed(1);
      console.log(`📊 Progress: ${progress}%`);
      console.log('');
      
      // Delay between slides
      await delay(2000);
      
    } catch (error) {
      console.log(`❌ Error on slide ${slideNum}:`, error);
      console.log('⏭️  Continuing to next slide...');
    }
  }
  
  console.log('🎉 CAPTURE COMPLETE!');
  console.log('====================');
  console.log('✅ All slides downloaded to your Downloads folder');
  console.log('📁 Files named: slide_001.png, slide_002.png, etc.');
}

// ===========================================
// STEP 3: Quick test function
// ===========================================

async function quickCapture() {
  console.log('🚀 QUICK CAPTURE - Current slide');
  
  try {
    // Find slide content (same working logic)
    const slideSelectors = [
      '[data-testid="design-canvas"]',
      '.canvas-container', 
      '[class*="canvas"]',
      'main > div:first-child'
    ];
    
    let captureElement = null;
    for (const selector of slideSelectors) {
      const element = document.querySelector(selector);
      if (element && element.offsetWidth > 500) {
        captureElement = element;
        break;
      }
    }
    
    if (!captureElement) {
      captureElement = document.querySelector('main');
    }
    
    const canvas = await html2canvas(captureElement, {
      useCORS: true,
      allowTaint: true,
      backgroundColor: null,
      scale: 2
    });
    
    const dataUrl = canvas.toDataURL('image/png', 1.0);
    const link = document.createElement('a');
    link.download = `canva_slide_test_${Date.now()}.png`;
    link.href = dataUrl;
    link.click();
    
    console.log('✅ Test slide downloaded!');
    
  } catch (error) {
    console.log('❌ Capture failed:', error);
  }
}

// ===========================================
// STEP 4: Instructions
// ===========================================

console.log(`
🎯 SIMPLE WORKING VERSION - INSTRUCTIONS:
=========================================

1. 📋 COPY & PASTE this script into Console
2. ⏳ WAIT for "html2canvas loaded successfully!" 
3. 🧪 TEST: quickCapture() - test current slide
4. 🚀 RUN: startSlideCapture() - capture all 77 slides

COMMANDS:
• quickCapture()      - Test current slide quality
• startSlideCapture() - Capture all 77 slides  

This is the WORKING version that gave you the good 
"Movement: Seal" screenshot quality.
`);